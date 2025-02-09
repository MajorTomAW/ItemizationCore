// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/InventoryManager.h"

#include "DisplayDebugHelpers.h"
#include "InventoryItemInstance.h"
#include "ItemizationCoreLog.h"
#include "ItemizationCoreStats.h"
#include "ActorComponents/EquipmentManager.h"
#include "Components/ItemComponentData_MaxStackSize.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/NetworkDelegates.h"
#include "Engine/NetConnection.h"
#include "Engine/Engine.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryManager)

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
	if (!IsValid(Actor))
	{
		return nullptr;
	}
	
	UInventoryManager* Mgr = nullptr;
	if (Actor)
	{
		Mgr = Actor->GetComponentByClass<UInventoryManager>();
	}

	if (Mgr == nullptr)
	{
		if (const APawn* Pawn = Cast<APawn>(Actor))
		{
			Mgr = Pawn->GetController() ? Pawn->GetController()->GetComponentByClass<UInventoryManager>() : nullptr;

			if (Mgr == nullptr)
			{
				Mgr = Pawn->GetPlayerState() ? Pawn->GetPlayerState()->GetComponentByClass<UInventoryManager>() : nullptr;
			}
		}
		else if (const AController* Controller = Cast<AController>(Actor))
		{
			Mgr = Controller->GetPawn() ? Controller->GetPawn()->GetComponentByClass<UInventoryManager>() : nullptr;

			if (Mgr == nullptr)
			{
				Mgr = Controller->GetPlayerState<APlayerState>() ? Controller->GetPlayerState<APlayerState>()->GetComponentByClass<UInventoryManager>() : nullptr;
			}
		}
	}

	return Mgr;
}

bool UInventoryManager::IsOwnerActorAuthoritative() const
{
	return !bCachedIsNetSimulated;
}

void UInventoryManager::ForceAvatarReplication()
{
	if (AActor* LocalAvatar = GetAvatarActor_Direct())
	{
		LocalAvatar->ForceNetUpdate();
	}
}

FInventoryItemEntryHandle UInventoryManager::GiveItem(const FInventoryItemEntry& ItemEntry, int32& Excess)
{
	FItemActionContextData ContextData = CreateItemActionContextData(ItemEntry);
	if (ContextData.InventoryManager == nullptr)
	{
		ContextData.InventoryManager = this;
	}
	
	return GiveItem(ItemEntry, ContextData, Excess);
}

FInventoryItemEntryHandle UInventoryManager::GiveItem(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& ContextData, int32& Excess)
{
	if (!IsValid(ItemEntry.Definition))
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to give an item with an invalid definition."),
			__FUNCTION__, *GetName());
		return FInventoryItemEntryHandle::NullHandle;
	}

	if (!IsOwnerActorAuthoritative())
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to give item (%s) on the client-side."),
			__FUNCTION__, *GetName(), *GetNameSafe(ItemEntry.Definition));
		return FInventoryItemEntryHandle::NullHandle;
	}
	
	// If locked, add to the pending-list.
	// The entry handle won't be regenerated when the item is actually added, so we can use it to refer to the item.
	if (InventoryScopeLockCount > 0)
	{
		ITEMIZATION_LOG(Verbose, TEXT("[%hs] (%s): Adding item (%s) to pending adds."),
			__FUNCTION__, *GetName(), *GetNameSafe(ItemEntry.Definition));
		ItemPendingAdds.Add(ItemEntry);
		return ItemEntry.Handle;
	}
	
	INVENTORY_LIST_SCOPE_LOCK();

	FItemActionContextData CurrentContext = ContextData;
	EvaluateCurrentContext(ItemEntry, CurrentContext);

	ITEMIZATION_LOG(Display, TEXT("[%hs] (%s): Giving Item [%s] %s. Stack Count: %d, Max Stack Size: %d"),
		__FUNCTION__, *GetName(), *ItemEntry.Handle.ToString(), *GetNameSafe(ItemEntry.Instance), CurrentContext.Delta, CurrentContext.MaxStackSize);

	return NativeGiveItem(ItemEntry, CurrentContext, Excess);
}

FInventoryItemEntryHandle UInventoryManager::GiveItem(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& ContextData)
{
	int32 Excess;
	return GiveItem(ItemEntry, ContextData, Excess);
}

FInventoryItemEntryHandle UInventoryManager::GiveItem(const FInventoryItemEntry& ItemEntry)
{
	int32 Excess;
	return GiveItem(ItemEntry, Excess);
}

FInventoryItemEntryHandle UInventoryManager::NativeGiveItem(
	const FInventoryItemEntry& ItemEntry, const FItemActionContextData& ContextData, int32& Excess)
{
	FInventoryItemEntryHandle LastHandle;

	FItemActionContextData& MutableContext = const_cast<FItemActionContextData&>(ContextData);
	
	// Try to find existing items first that we could fill up first before creating a new instance.
	if (MutableContext.MaxStackSize > 1)
	{
		for (FInventoryItemEntry& Item : InventoryList.Items)
		{
			int32 OutExcess;
			const bool bCouldCombine = CombineItemEntries(ItemEntry, Item, MutableContext, OutExcess);

			if (!bCouldCombine)
			{
				continue;
			}

			MutableContext.Delta = OutExcess;
			LastHandle = Item.Handle;

			// Mark dirty for replication
			MarkItemEntryDirty(Item, true);
		}
	}

	// Create a new stack if we couldn't fill up an existing one.
	while (MutableContext.Delta > 0)
	{
		// Check if we can create a new stack.
		if (!CanCreateNewStack(ItemEntry, MutableContext))
		{
			break;
		}
		
		const int32 Delta = FMath::Min(MutableContext.Delta, MutableContext.MaxStackSize);
		MutableContext.Delta -= Delta;

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

	Excess = FMath::Max(0, MutableContext.Delta);
	
	return LastHandle;
}

FInventoryItemEntryHandle UInventoryManager::K2_GiveItem(UItemDefinition* ItemDefinition, int32 StackCount, int32& Excess)
{
	// Temp: Create a non-evaluated context data. Will be evaluated in GiveItem().
	FItemActionContextData ContextData;
	ContextData.InventoryManager = this;
	ContextData.Instigator = GetOwnerActor();

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

bool UInventoryManager::RemoveItem(const FInventoryItemEntryHandle& Handle, int32 StackCount)
{
	if (!Handle.IsValid())
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to remove an item with an invalid handle."),
			__FUNCTION__, *GetName());
		return false;
	}

	if (!IsOwnerActorAuthoritative())
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to remove item (%s) on the client-side."),
			__FUNCTION__, *GetName(), *Handle.ToString());

		return false;
	}

	const TFunction<bool(const FInventoryItemEntry&)> Pred = [Handle](const FInventoryItemEntry& Other)->bool
	{
		return Other.Handle == Handle;
	};

	return RemoveItemByPredicate(Pred, StackCount);
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

FInventoryItemEntry* UInventoryManager::FindItemEntryFromDefinition(UItemDefinition* ItemDefinition) const
{
	SCOPE_CYCLE_COUNTER(STAT_FindItemEntryFromHandle);

	for (const FInventoryItemEntry& ItemEntry : InventoryList.Items)
	{
		if (ItemEntry.Definition == nullptr)
		{
			continue;
		}

		if (ItemEntry.Definition == ItemDefinition)
		{
			return const_cast<FInventoryItemEntry*>(&ItemEntry);
		}
	}

	return nullptr;
}

int32 UInventoryManager::GetCurrentStackCount(FInventoryItemEntryHandle Handle) const
{
	if (const FInventoryItemEntry* ItemEntry = FindItemEntryFromHandle(Handle))
	{
		return ItemEntry->StackCount;
	}

	return 0;
}

FItemActionContextData UInventoryManager::CreateItemActionContextData(const FInventoryItemEntry& Item)
{
	FItemActionContextData Context(Item);
	Context.InventoryManager = this;
	return Context;
}

void UInventoryManager::GetAllItemHandles(TArray<FInventoryItemEntryHandle>& OutHandles) const
{
	// Reset the output array
	OutHandles.Empty(InventoryList.Items.Num());

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
		for (UInventoryItemInstance* Instance : GetReplicatedItemInstances_Mutable())
		{
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

	Params.Condition = COND_None;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OwnerActor, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, AvatarActor, Params);
	
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

	if (!InventoryData.IsValid())
	{
		InventoryData = MakeShareable(new FItemizationCoreInventoryData());
	}
}

void UInventoryManager::InitializeComponent()
{
	Super::InitializeComponent();

	AActor* Owner = GetOwner();
	InitInventorySystem(Owner, Owner);
}

void UInventoryManager::UninitializeComponent()
{
	Super::UninitializeComponent();
}

bool UInventoryManager::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
#if SUBOBJECT_TRANSITION_VALIDATION
	if (UActorChannel::CanIgnoreDeprecatedReplicateSubObjects())
	{
		return false;
	}
#endif

	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (UInventoryItemInstance* Instance : GetReplicatedItemInstances())
	{
		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
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

			for (FInventoryItemEntry& ItemEntry : ActiveChange.Adds)
			{
				int32 Excess;
				GiveItem(ItemEntry, Excess);
			}

			for (const FInventoryItemEntryHandle& Handle : ActiveChange.Removes)
			{
				RemoveItem(Handle);
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
		bool bCanClearItem = true;
		for (const FItemComponentData* Comp : Entry.Definition->GetItemComponents())
		{
			if (!Comp->IncludeInClearAll())
			{
				bCanClearItem = false;
			}
		}

		if (bCanClearItem)
		{
			OnRemoveItem(Entry);
		}
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
	if (UInventoryItemInstance* Instance = ItemEntry.Instance)
	{
		Instance->OnRemovedFromInventory(ItemEntry, Instance->CurrentInventoryData);

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
		Instance->OnAddedToInventory(ItemEntry, InventoryData.Get());
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

bool UInventoryManager::RemoveItemByPredicate(const TFunctionRef<bool(const FInventoryItemEntry& Other)>& Predicate, int32 StackCount)
{
	if (StackCount <= 0)
	{
		StackCount = TNumericLimits<int32>::Max();
	}

	for (int Idx = 0; Idx < ItemPendingAdds.Num(); ++Idx)
	{
		if (Predicate(ItemPendingAdds[Idx]))
		{
			ItemPendingAdds.RemoveAtSwap(Idx, 1, EAllowShrinking::No);
			return true;
		}
	}

	bool bRemovedAtLeastOne = false;
	for (auto It = InventoryList.Items.CreateIterator(); It; ++It)
	{
		// No more items to remove?
		if (StackCount <= 0)
		{
			return true;
		}

		FInventoryItemEntry& ItemEntry = *It;
		check(ItemEntry.Instance);

		if (!Predicate(ItemEntry))
		{
			ITEMIZATION_LOG(Verbose, TEXT("Predicate failed for item [%s] %s."), *ItemEntry.Handle.ToString(), *GetNameSafe(ItemEntry.Instance));
			continue;
		}

		const int32 Delta = FMath::Min(StackCount, ItemEntry.StackCount);
		ItemEntry.StackCount -= Delta;
		StackCount -= Delta;

		bRemovedAtLeastOne = true;

		ITEMIZATION_LOG(Display, TEXT("Removing Item [%s] %s. Stack Count: %d"),
			*ItemEntry.Handle.ToString(), *GetNameSafe(ItemEntry.Instance), ItemEntry.StackCount);

		MarkItemEntryDirty(ItemEntry, true);

		// If the stack count is now 0, remove the item.
		if (ItemEntry.StackCount <= 0)
		{
			bool bCanClearItem = true;
			for (const FItemComponentData* Comp : ItemEntry.Definition->GetItemComponents())
			{
				if (!Comp->CanClearItem(ItemEntry))
				{
					bCanClearItem = false;
					break;
				}
			}

			if (bCanClearItem)
			{
				// Scope lock to ensure the item is removed properly.
				INVENTORY_LIST_SCOPE_LOCK();
				OnRemoveItem(ItemEntry);

				It.RemoveCurrent();
				InventoryList.MarkArrayDirty();
			}
		}
	}

	return bRemovedAtLeastOne;
}

void UInventoryManager::AddReplicatedItemInstance(UInventoryItemInstance* ItemInstance)
{
	TArray<TObjectPtr<UInventoryItemInstance>>& ReplicatedInstances = GetReplicatedItemInstances_Mutable();
	if (ReplicatedInstances.Find(ItemInstance) == INDEX_NONE)
	{
		ReplicatedInstances.Add(ItemInstance);
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			const ELifetimeCondition LifetimeCondition = bReplicateInventoryItemsToSimulatedProxies ? COND_None : COND_ReplayOrOwner;
			AddReplicatedSubObject(ItemInstance, LifetimeCondition);
		}	
	}
}

void UInventoryManager::RemoveReplicatedItemInstance(UInventoryItemInstance* ItemInstance)
{
	const bool bWasRemoved = GetReplicatedItemInstances_Mutable().RemoveSingle(ItemInstance) > 0;
	
	if (bWasRemoved && IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

void UInventoryManager::SetOwnerActor(AActor* NewOwnerActor)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OwnerActor, this);
	if (OwnerActor)
	{
		OwnerActor->OnDestroyed.RemoveDynamic(this, &ThisClass::OnOwnerActorDestroyed);
	}

	OwnerActor = NewOwnerActor;

	if (OwnerActor)
	{
		OwnerActor->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnOwnerActorDestroyed);
	}
}

void UInventoryManager::SetAvatarActor_Direct(AActor* NewAvatarActor)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, AvatarActor, this);
	if (AvatarActor)
	{
		AvatarActor->OnDestroyed.RemoveDynamic(this, &ThisClass::OnAvatarActorDestroyed);
	}

	AvatarActor = NewAvatarActor;

	if (AvatarActor)
	{
		AvatarActor->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnAvatarActorDestroyed);
	}
}

void UInventoryManager::SetAvatarActor(AActor* NewAvatarActor)
{
	check(InventoryData.IsValid());
	InitInventorySystem(GetOwnerActor(), NewAvatarActor);
}


AActor* UInventoryManager::GetAvatarActor() const
{
	check(InventoryData.IsValid());
	return InventoryData->AvatarActor.Get();
}

void UInventoryManager::InitInventorySystem(AActor* InOwnerActor, AActor* InAvatarActor)
{
	check(InventoryData.IsValid());
	const bool bWasAvatarNull = InventoryData->AvatarActor == nullptr;
	const bool bAvatarChanged = InAvatarActor != InventoryData->AvatarActor;

	//@TODO: Manually check if the avatar is a controller or player state and retrieve its pawn ?
	if (const AController* C = Cast<AController>(InAvatarActor))
	{
		InAvatarActor = C->GetPawn();
	}
	else if (const APlayerState* PS = Cast<APlayerState>(InAvatarActor))
	{
		InAvatarActor = PS->GetPawn();
	}

	InventoryData->InitFromActor(InOwnerActor, InAvatarActor, this);
	SetOwnerActor(InOwnerActor);

	// Caching previous avatar to check against
	const AActor* OldAvatar = GetAvatarActor_Direct();
	SetAvatarActor_Direct(InAvatarActor);

	// Notify all our items that the avatar has changed.
	if (bAvatarChanged)
	{
		INVENTORY_LIST_SCOPE_LOCK();

		for (FInventoryItemEntry& ItemEntry : InventoryList.Items)
		{
			if (ItemEntry.Instance == nullptr)
			{
				continue;
			}

			ItemEntry.Instance->OnAvatarSet(ItemEntry, InventoryData.Get());
		}
	}

	OnInventorySystemInitialized();
}

void UInventoryManager::ClearInventoryData()
{
	check(InventoryData.IsValid());
	InventoryData->ClearInventoryData();
	SetOwnerActor(nullptr);
	SetAvatarActor_Direct(nullptr);
}

void UInventoryManager::OnInventorySystemInitialized()
{
}

void UInventoryManager::OnRep_OwnerActor()
{
	check(InventoryData.IsValid());
	
	AActor* LocalOwnerActor = GetOwnerActor();
	AActor* LocalAvatarActor = GetAvatarActor_Direct();

	if (LocalOwnerActor != InventoryData->OwnerActor ||
		LocalAvatarActor != InventoryData->AvatarActor)
	{
		if (LocalOwnerActor != nullptr)
		{
			InitInventorySystem(LocalOwnerActor, LocalAvatarActor);
		}
		else
		{
			ClearInventoryData();
		}
	}
}

void UInventoryManager::OnAvatarActorDestroyed(AActor* InActor)
{
	if (InActor == AvatarActor)
	{
		AvatarActor = nullptr;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, AvatarActor, this);
	}
}

void UInventoryManager::OnOwnerActorDestroyed(AActor* InActor)
{
	if (InActor == OwnerActor)
	{
		OwnerActor = nullptr;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OwnerActor, this);
	}
}

void UInventoryManager::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	if (DisplayInfo.IsDisplayOn(TEXT("Itemization")))
	{
		if (const UInventoryManager* Mgr = GetInventoryManager(HUD->GetCurrentDebugTargetActor()))
		{
			Mgr->DisplayDebug(Canvas, DisplayInfo, YL, YPos);
		}
	}
}

void UInventoryManager::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) const
{
	if (DebugDisplay.IsDisplayOn(FName(TEXT("Items"))))
	{
		
	}
}
