// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/InventoryManager.h"

#include "InventoryItemInstance.h"
#include "ItemDefinition.h"
#include "ItemizationCoreLog.h"
#include "ItemizationCoreStats.h"
#include "Components/ItemComponentData_MaxStackSize.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

static bool bReplicateInventoryItemsToSimulatedProxies = false;
static FAutoConsoleVariableRef CVarReplicateInventoryItemsToOwnerOnly
(
	TEXT("ItemizationCore.Inventory.ReplicateItemsToSimulatedProxies"),
	bReplicateInventoryItemsToSimulatedProxies,
	TEXT("Default: False. When false, inventory items replicate to AutonomousProxies only, not SimulatedProxies")
);

UInventoryManager::UInventoryManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
	
	bCachedIsNetSimulated = false;
	
	InventoryScopeLockCount = 0;
	bInventoryPendingClearAll = false;

	SetIsReplicatedByDefault(true);
}

UInventoryManager* UInventoryManager::GetInventoryManager(AActor* Actor)
{
	void* Mgr = nullptr;
	if (Actor)
	{
		Mgr = Actor->GetComponentByClass<UInventoryManager>();
	}

	if (Mgr == nullptr)
	{
		if (const APawn* Pawn = Cast<APawn>(Actor))
		{
			Mgr = Pawn->GetController()->GetComponentByClass<UInventoryManager>();

			if (Mgr == nullptr)
			{
				Mgr = Pawn->GetPlayerState()->GetComponentByClass<UInventoryManager>();
			}
		}
		else if (const AController* Controller = Cast<AController>(Actor))
		{
			Mgr = Controller->GetPawn()->GetComponentByClass<UInventoryManager>();

			if (Mgr == nullptr)
			{
				Mgr = Controller->GetPlayerState<APlayerState>()->GetComponentByClass<UInventoryManager>();
			}
		}
	}

	return (UInventoryManager*)Mgr;
}

bool UInventoryManager::IsOwnerActorAuthoritative() const
{
	return !bCachedIsNetSimulated;
}

FInventoryItemEntryHandle UInventoryManager::GiveItem(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& Context, int32& Excess)
{
	if (!IsValid(ItemEntry.Definition))
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to give an item with an invalid definition."), __FUNCTION__, *GetName());
		return FInventoryItemEntryHandle();
	}

	if (!IsOwnerActorAuthoritative())
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to give item (%s) on the client-side."), __FUNCTION__, *GetName(), *GetNameSafe(ItemEntry.Definition));
		return FInventoryItemEntryHandle();
	}
	
	// If locked, add to the pending-list.
	// The entry handle won't be regenerated when the item is actually added, so we can use it to refer to the item.
	if (InventoryScopeLockCount > 0)
	{
		ITEMIZATION_LOG(Verbose, TEXT("[%hs] (%s): Adding item (%s) to pending adds."), __FUNCTION__, *GetName(), *GetNameSafe(ItemEntry.Definition));
		ItemPendingAdds.Add(ItemEntry);
		return ItemEntry.Handle;
	}
	
	INVENTORY_LIST_SCOPE_LOCK();

	FItemActionContextData CurrentContext = Context;
	EvaluateCurrentContext(ItemEntry, CurrentContext);

	// See if we can stack the item with an existing item.
	FInventoryItemEntryHandle LastHandle;

	ITEMIZATION_LOG(Display, TEXT("[%hs] (%s): Giving Item [%s] %s. Stack Count: %d, Max Stack Size: %d"),
		__FUNCTION__, *GetName(), *ItemEntry.Handle.ToString(), *GetNameSafe(ItemEntry.Instance), CurrentContext.Delta, CurrentContext.MaxStackSize);

	// Try to find existing items first that we could fill up first before creating a new instance.
	if (CurrentContext.MaxStackSize > 1)
	{
		for (FInventoryItemEntry& Item : InventoryList.Items)
		{
			int32 OutExcess;
			const bool bCouldCombine = CombineItemEntries(ItemEntry, Item, CurrentContext, OutExcess);

			if (!bCouldCombine)
			{
				continue;
			}

			CurrentContext.Delta = OutExcess;
			LastHandle = Item.Handle;

			// Mark dirty for replication
			MarkItemEntryDirty(Item, true);
		}
	}

	// Create a new stack if we couldn't fill up an existing one.
	while (CurrentContext.Delta > 0)
	{
		// Check if we can create a new stack.
		if (!CanCreateNewStack(ItemEntry, CurrentContext))
		{
			break;
		}
		
		const int32 Delta = FMath::Min(CurrentContext.Delta, CurrentContext.MaxStackSize);
		CurrentContext.Delta -= Delta;

		FInventoryItemEntry EntryCopy = ItemEntry;
		EntryCopy.StackCount = Delta;

		FInventoryItemEntry& NewItem = InventoryList.Items.Add_GetRef(EntryCopy);

		// Generate a new handle for each new stack
		NewItem.Handle.GenerateNewHandle();
		
		LastHandle = NewItem.Handle;

		OnGiveItem(NewItem);
		MarkItemEntryDirty(NewItem, true);

		ITEMIZATION_LOG(Verbose, TEXT("[%hs] (%s): Creating new Item Stack [%s] %s. Stack Count: %d"),
			__FUNCTION__, *GetName(), *NewItem.Handle.ToString(), *GetNameSafe(NewItem.Instance), NewItem.StackCount);
	}

	Excess = FMath::Max(0, CurrentContext.Delta);
	return LastHandle;
}

FInventoryItemEntryHandle UInventoryManager::K2_GiveItem(UItemDefinition* ItemDefinition, int32 StackCount, int32& Excess)
{
	// Temp: Create a non-evaluated context data. Will be evaluated in GiveItem().
	FItemActionContextData ContextData;
	ContextData.InventoryManager = this;
	ContextData.Instigator = this;

	// Build and validate item entry
	FInventoryItemEntry ItemEntry = BuildItemEntryFromDefinition(ItemDefinition, StackCount, ContextData);

	// Validate the item definition
	if (!IsValid(ItemEntry.Definition))
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to give an item with an invalid definition."), __FUNCTION__, *GetName());
		return FInventoryItemEntryHandle();
	}

	// Give the item and return its handle.
	// Will run validation and authority checks.
	return GiveItem(ItemEntry, ContextData, Excess);
}

FInventoryItemEntry UInventoryManager::BuildItemEntryFromDefinition(UItemDefinition* ItemDefinition, int32 StackCount, const FItemActionContextData& ContextData)
{
	// Validate the item definition
	if (!ensure(ItemDefinition))
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to build an item entry from an invalid item definition."), __FUNCTION__, *GetName());
		return FInventoryItemEntry();
	}

	return FInventoryItemEntry(ItemDefinition, StackCount, ContextData.Instigator.Get());
}

bool UInventoryManager::CombineItemEntries(const FInventoryItemEntry& ThisEntry, FInventoryItemEntry& OtherEntry, FItemActionContextData& Context, int32& Excess)
{
	SCOPE_CYCLE_COUNTER(STAT_CanCombineItemEntries);
	
	// Only items of the same definition can be combined.
	if (ThisEntry.Definition != OtherEntry.Definition)
	{
		return false;
	}

	for (const FItemComponentData* Component : OtherEntry.Definition->GetItemComponents())
	{
		if (!Component->CanCombineItemStacks(ThisEntry, OtherEntry, Context))
		{
			return false;
		}
	}

	// Calculate the excess amount of items that couldn't be added to the stack.
	Excess = ThisEntry.StackCount + OtherEntry.StackCount - Context.MaxStackSize;
	OtherEntry.StackCount = FMath::Min(Context.MaxStackSize, ThisEntry.StackCount + OtherEntry.StackCount);
	return true;
}

bool UInventoryManager::CanCreateNewStack(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& Context) const
{
	SCOPE_CYCLE_COUNTER(STAT_CanCreateNewStack);
	
	if (ItemEntry.Definition == nullptr)
	{
		return false;
	}

	for (const FItemComponentData* Component : ItemEntry.Definition->GetItemComponents())
	{
		if (!Component->CanCreateNewStack(ItemEntry, Context))
		{
			return false;
		}
	}

	return true;
}

void UInventoryManager::EvaluateCurrentContext(const FInventoryItemEntry& ItemEntry, FItemActionContextData& OutContext) const
{
	SCOPE_CYCLE_COUNTER(STAT_EvaluateCurrentItemContext);
	
	if (!ensure(ItemEntry.Definition))
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to evaluate the context of an item entry with an invalid item definition."), __FUNCTION__, *GetName());
		return;
	}

	OutContext.Delta = ItemEntry.StackCount;

	for (const auto& Proxy : ItemEntry.Definition->ItemComponents)
	{
		const FItemComponentData* Component = Proxy.Component.GetPtr<FItemComponentData>();
		if (Component == nullptr)
		{
			// Maybe log an error here? todo
			continue;
		}

		Component->EvaluateContext(ItemEntry, OutContext);
	}

	ITEMIZATION_LOG(Display, TEXT("Evaluated Item Context: %s"), *OutContext.ToString());
}

void UInventoryManager::MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange)
{
	if (IsOwnerActorAuthoritative())
	{
		if (ItemEntry.Instance == nullptr || bWasAddOrChange)
		{
			InventoryList.MarkItemDirty(ItemEntry);
		}
		else
		{
			InventoryList.MarkArrayDirty();
		}
	}
	else
	{
		// Client-side, mark the array dirty so it will be replicated.
		InventoryList.MarkArrayDirty();
	}
}


void UInventoryManager::GetAllItemsOfType(TArray<FInventoryItemEntry*>* OutItems, const UItemDefinition* ItemDefinition)
{
	InventoryList.GetItemsOfType(OutItems, ItemDefinition);
}

FInventoryItemEntry* UInventoryManager::FindItemEntryFromHandle(FInventoryItemEntryHandle Handle, EConsiderPending ConsiderPending) const
{
	SCOPE_CYCLE_COUNTER(STAT_FindItemEntryFromHandle);
	
	for (const FInventoryItemEntry& ItemEntry : InventoryList.Items)
	{
		if (ItemEntry.Handle != Handle)
		{
			continue;
		}

		if (ItemEntry.bPendingRemove && !EnumHasAnyFlags(ConsiderPending, EConsiderPending::PendingRemove))
		{
			continue;
		}

		return const_cast<FInventoryItemEntry*>(&ItemEntry);
	}

	if (EnumHasAnyFlags(ConsiderPending, EConsiderPending::PendingAdd))
	{
		for (const FInventoryItemEntry& ItemEntry : ItemPendingAdds)
		{
			if (ItemEntry.Handle != Handle)
			{
				continue;
			}

			if (ItemEntry.bPendingRemove && !EnumHasAnyFlags(ConsiderPending, EConsiderPending::PendingRemove))
			{
				continue;
			}

			return const_cast<FInventoryItemEntry*>(&ItemEntry);
		}
	}

	return nullptr;
}

void UInventoryManager::GetAllItemHandles(TArray<FInventoryItemEntryHandle>& OutHandles) const
{
	// Reset the output array
	OutHandles.Empty();

	// Iterate through the inventory list and add all handles to the output array
	for (const FInventoryItemEntry& ItemEntry : InventoryList.Items)
	{
		OutHandles.Add(ItemEntry.Handle);
	}
}

void UInventoryManager::PreNetReceive()
{
	// Update the cached IsNetSimulated value here if this component is still considered authoritative.
	// Even though the value is also cached in OnRegister and BeginPlay, clients may
	// receive properties before OnBeginPlay, so this ensures the role is correct for that case.
	if (!bCachedIsNetSimulated)
	{
		CacheIsNetSimulated();
	}
}

void UInventoryManager::BeginPlay()
{
	Super::BeginPlay();

	CacheIsNetSimulated();
}

void UInventoryManager::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (const FInventoryItemEntry& ItemSpec : InventoryList.Items)
		{
			UInventoryItemInstance* Instance = ItemSpec.Instance;

			if (IsValid(Instance))
			{
				const ELifetimeCondition LifetimeCondition = bReplicateInventoryItemsToSimulatedProxies ? COND_None : COND_ReplayOrOwner;
				AddReplicatedSubObject(Instance, LifetimeCondition);
			}
		}
	}
}

void UInventoryManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_ReplayOrOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryList, Params);
	
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UInventoryManager::GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const
{
	Super::GetReplicatedCustomConditionState(OutActiveState);
}

void UInventoryManager::OnRegister()
{
	Super::OnRegister();

	CacheIsNetSimulated();

	InventoryList.RegisterWithOwner(this);
}

void UInventoryManager::IncrementInventoryListLock()
{
	InventoryScopeLockCount++;
}

void UInventoryManager::DecrementInventoryListLock()
{
	if (--InventoryScopeLockCount == 0)
	{
		if (bInventoryPendingClearAll)
		{
			ClearAllItems();
			
			// When there are pending Adds but also pending clear all,
			// we prioritize the clear-all since ClearAllItems() based on an assumption
			// that the clear-all is likely end-of-life cleanup. There may be cases where someone intentionally calls ClearAllItems() and
			// then GiveItem() within one inventory scope lock.
			if (ItemPendingAdds.Num() > 0)
			{
				ITEMIZATION_LOG(Warning, TEXT("GiveItem and ClearAllItems were both called within an inventory scope lock. Prioritizing clear all items by ignoring pending adds."))
				ItemPendingAdds.Reset();
			}

			// Pending removes are no longer relevant since all items have been removed
			ItemPendingRemoves.Reset();
		}
		else if (ItemPendingAdds.Num() > 0 || ItemPendingRemoves.Num() > 0)
		{
			FInventoryListLockActiveChange ActiveChange(*this, ItemPendingAdds, ItemPendingRemoves);

			for (FInventoryItemEntry& Spec : ActiveChange.Adds)
			{
				
			}

			for (FInventoryItemEntryHandle& Handle : ActiveChange.Removes)
			{
				
			}
		}
	}
}

void UInventoryManager::CacheIsNetSimulated()
{
	bCachedIsNetSimulated = IsNetSimulating();
}

void UInventoryManager::ClearAllItems()
{
	// If this is called inside an inventory scope lock, postpone the workload until the end of the scope.
	if (InventoryScopeLockCount > 0)
	{
		bInventoryPendingClearAll = true;
		return;
	}

	if (!IsOwnerActorAuthoritative())
	{
		ITEMIZATION_LOG(Error, TEXT("Attempted to call ClearAllItems() without authority."));
		return;
	}

	// Note we aren't marking any old items pending kill. This shouldn't matter since they will be garbage collected.
	INVENTORY_LIST_SCOPE_LOCK();
	for (FInventoryItemEntry& Entry : InventoryList.Items)
	{
		OnRemoveItem(Entry);
	}

	InventoryList.Items.Empty(InventoryList.Items.Num());
	InventoryList.MarkArrayDirty();
}

void UInventoryManager::OnRep_InventoryList()
{
	for (FInventoryItemEntry& ItemEntry : InventoryList.Items)
	{
		const UInventoryItemInstance* Instance = ItemEntry.Instance;
		if (Instance == nullptr)
		{
			// Queue up another call to make sure this gets run again, as the instance hasn't been replicated yet.
			GetWorld()->GetTimerManager().SetTimer(OnRep_InventoryListTimerHandle, this, &ThisClass::OnRep_InventoryList, 0.5f);
			return;
		}
	}
}

#if WITH_GAMEPLAY_MESSAGE_ROUTER
void UInventoryManager::BroadcastInventoryChangeMessage(UInventoryItemInstance* ItemThatChanged, const int32 OldStackCount, const int32 NewStackCount) const
{
	InventoryList.BroadcastInventoryChangeMessage(ItemThatChanged, OldStackCount, NewStackCount);
}
#endif

void UInventoryManager::OnRemoveItem(FInventoryItemEntry& ItemEntry)
{
	ensureMsgf(InventoryScopeLockCount > 0, TEXT("%hs called without an inventory list lock. It can produce side effects and should be locked to pin the Entry argument."), __func__);

	if (!ItemEntry.Instance)
	{
		return;
	}

	ITEMIZATION_LOG(Display, TEXT("%s: Removing Item [%s] %s. Stack Count: %d"), *GetName(), *ItemEntry.Handle.ToString(), *GetNameSafe(ItemEntry.Instance), ItemEntry.StackCount);

	// Notify the item that has been removed.
	UInventoryItemInstance* Instance = ItemEntry.Instance;
	if (Instance)
	{
		Instance->OnRemovedFromInventory(this, ItemEntry);

		// Make sure we remove this before marking it as garbage.
		if (GetOwnerRole() == ROLE_Authority)
		{
			// Only destroy if we're authoritative.
			// Can't destroy on clients or replication will fail since the item will be marked as pending kill.
			RemoveReplicatedItemInstance(Instance);
		}

		Instance->MarkAsGarbage();
	}

	ItemEntry.Instance = nullptr;
}

void UInventoryManager::OnGiveItem(FInventoryItemEntry& ItemEntry)
{
	if (!ItemEntry.Definition)
	{
		return;
	}

	UInventoryItemInstance* Instance = ItemEntry.Instance;
	if (Instance == nullptr)
	{
		// Create a new instance if we should.
		if (ShouldCreateNewInstanceOfItem(ItemEntry, ItemEntry.StackCount))
		{
			ItemEntry.Instance = CreateNewInstanceOfItem(ItemEntry);
			Instance = ItemEntry.Instance;
		}
	}

	if (ensure(Instance))
	{
		Instance->OnAddedToInventory(this, ItemEntry);
	}
}

bool UInventoryManager::ShouldCreateNewInstanceOfItem(const FInventoryItemEntry& ItemEntry, const uint32 InStackCount) const
{
	return true;
}

UInventoryItemInstance* UInventoryManager::CreateNewInstanceOfItem(FInventoryItemEntry& ItemEntry)
{
	check(ItemEntry.Instance == nullptr);

	const UItemDefinition* Definition = ItemEntry.Definition;
	check(Definition);

	AActor* Owner = GetOwner();
	check(Owner);

	UInventoryItemInstance* NewInstance = NewObject<UInventoryItemInstance>(Owner, Definition->GetDefaultInstanceClass());
	check(NewInstance);
	
	ItemEntry.Instance = NewInstance;

	AddReplicatedItemInstance(NewInstance);
	return NewInstance;
}

void UInventoryManager::AddReplicatedItemInstance(UInventoryItemInstance* ItemInstance)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		const ELifetimeCondition LifetimeCondition = bReplicateInventoryItemsToSimulatedProxies ? COND_None : COND_ReplayOrOwner;
		AddReplicatedSubObject(ItemInstance, LifetimeCondition);
	}
}

void UInventoryManager::RemoveReplicatedItemInstance(UInventoryItemInstance* ItemInstance)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

void UInventoryManager::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
}
