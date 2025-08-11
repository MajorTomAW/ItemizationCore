// Author: Tom Werner (MajorT), 2025


#include "Inventory/InventoryBase.h"

#include "ItemizationCoreLogChannels.h"
#include "ItemizationCoreStats.h"
#include "ItemizationCoreSettings.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Inventory/Operations/InventoryOp.h"
#include "ItemizationCore/Public/Inventory/Operations/InventoryOp_ItemAction.h"
#include "Items/InventoryItemInstance.h"
#include "Items/ItemDefinitionBase.h"
#include "Items/Data/ItemComponentData_Traits.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryBase)


namespace InventoryCVars
{
	static TAutoConsoleVariable<float> CVarInventoryNetPriority(
		TEXT("Inventory.MaxInventoryOperationLifetime"),
		10.0f,
		TEXT("How long a single inventory operation should be kept alive before it will get removed from the cache. "),
		ECVF_Default);
}

AInventoryBase::AInventoryBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	SetNetDormancy(DORM_Awake);
	bReplicates = true;
	bAlwaysRelevant = true;
	bNetLoadOnClient = true;
	bNetUseOwnerRelevancy = true;
	bReplicateUsingRegisteredSubObjectList = true;
	NetPriority = 3.0f;
	
	SetReplicatingMovement(false);
	SetNetUpdateFrequency(10.0f);
	SetMinNetUpdateFrequency(2.f);
	SetNetCullDistanceSquared(225000000.0f);

	SetHidden(false);
	SetCanBeDamaged(false);
	bEnableAutoLODGeneration = false;

	bAllowTickBeforeBeginPlay = false;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AInventoryBase::BeginPlay()
{
	Super::BeginPlay();

	// Send even for potential game feature actions
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
		this, UGameFrameworkComponentManager::NAME_GameActorReady);
}

void AInventoryBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// And unregister now !!
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	UWorld const* World = GetWorld();
	
	// Stop our timer that clears timed out or finished requests
	if (IsValid(World))
	{
		World->GetTimerManager().ClearTimer(FetchOpValidnessTimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

void AInventoryBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool AInventoryBase::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

bool AInventoryBase::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

void AInventoryBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Register us a receiver so we can add component requests to this shit.
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);

	// Start our timer that clears timed out or finished requests
	GetWorld()->GetTimerManager().SetTimer(
		FetchOpValidnessTimerHandle,
		FTimerDelegate::CreateUObject(this, &ThisClass::FetchOpValidness),
		InventoryCVars::CVarInventoryNetPriority->GetFloat(),
		true, 5.f);
}

FInventoryItemEntry AInventoryBase::CreateItemEntry(const FCreateItemEntryParams& InParams)
{
	// Build and validate item entry
	if (!ensure(InParams.ItemDefinition.IsValid()))
	{
		return FInventoryItemEntry();
	}

	return FInventoryItemEntry(InParams.ItemDefinition.Get(), InParams.SourceObject.Get(), InParams.StackSize);
}

TInventoryOpPtr<FInventoryOp_ItemAction> AInventoryBase::GiveItem(
	FInventoryOp_ItemAction::FParams&& Params,
	const FCreateItemEntryParams& CreateItemParams)
{
	check(CreateItemParams.ItemDefinition.IsValid());

	if (!HasAuthority())
	{
		return nullptr;
	}

	//@TODO: If locked, add to the pending list

	// Create the item entry from the given params
	FInventoryItemEntry NewItemEntry = CreateItemEntry(CreateItemParams);
	Params.ItemEntry = &NewItemEntry;

	// Evaluate the item entry
	EvaluateItemEntry(Params);

#if ENABLE_DRAW_DEBUG && !UE_BUILD_SHIPPING
	const int32 MaxStackSize = Params.ItemEntry->GetStatValue(Itemization::Tags::TAG_ItemStat_MaxStackSize);

	ITEMIZATION_LOG("Giving item '%s' [%s]\tSize: %d/%d\tSource: %s",
		*GetNameSafe(Params.ItemEntry->ItemDefinition),
		*Params.ItemEntry->ItemHandle.ToString(),
		Params.Delta,
		MaxStackSize,
		*GetNameSafe(Params.ItemEntry->SourceObject.Get()))

	Params.ItemEntry->DebugPrintStats();
#endif

	// Create the operation
	TInventoryOpRef<FInventoryOp_ItemAction> NewOp
		= OpCache.MakeSharedOp<FInventoryOp_ItemAction>(MoveTemp(Params));

	// Call the native version to actually give the item
	NativeGiveItem(NewOp);

	return NewOp;
}

TInventoryOpPtr<FInventoryOp_RemoveItem> AInventoryBase::RemoveItem(FInventoryOp_RemoveItem::FParams&& Params)
{
	check(Params.HasValidFilterFunc());

	if (!HasAuthority())
	{
		return nullptr;
	}

	//@TODO: If locked, add to the pending removal list

	// Create the operation
	TInventoryOpRef<FInventoryOp_RemoveItem> NewOp
		= OpCache.MakeSharedOp<FInventoryOp_RemoveItem>(MoveTemp(Params));

	// Call the native version to actually remove the item
	NativeRemoveItem(NewOp);

	return NewOp;
}

/*FInventoryItemHandle AInventoryBase::GiveItem(TInventoryOpRef<FInventoryOp_ItemAction> Action)
{
	check(Action->Params.ItemEntry);
	FInventoryOp_ItemAction::FParams& Params = Action->Params;

	//@TODO: If locked, add to pending list

	// Evaluate the item entry
	EvaluateItemEntry(Params);
	const int32 MaxStackSize = Params.ItemEntry->GetStatValue(Itemization::Tags::TAG_ItemStat_MaxStackSize);

	
	ITEMIZATION_LOG_NET("Giving item [%s] %s\tSize: %d/%d\tSource: %s",
		*Params.ItemEntry->ItemHandle.ToString(),
		*GetNameSafe(Params.ItemEntry->ItemDefinition),
		Params.Delta,
		MaxStackSize,
		*GetNameSafe(Params.ItemEntry->SourceObject.Get()));
	Params.ItemEntry->DebugPrintStats();

	return NativeGiveItem(Action);
}*/

FInventoryItemEntry* AInventoryBase::FindItemEntryByHandle(const FInventoryItemHandle& ItemHandle) const
{
	SCOPE_CYCLE_COUNTER(STAT_FindItemEntryByHandle);

	for (const FInventoryItemEntry& ItemEntry : InventoryList.Items)
	{
		if (ItemEntry.ItemHandle != ItemHandle)
		{
			continue;
		}

		// Skip items that are pending removal
		if (ItemEntry.bPendingRemove)
		{
			continue;
		}

		return &const_cast<FInventoryItemEntry&>(ItemEntry);
	}

	return nullptr;
}

TScriptInterface<IInventoryItemInstanceInterface> AInventoryBase::FindItemInstanceByHandle(const FInventoryItemHandle& ItemHandle) const
{
	SCOPE_CYCLE_COUNTER(STAT_FindItemInstanceByHandle);

	if (const FInventoryItemEntry* ItemEntry = FindItemEntryByHandle(ItemHandle))
	{
		return ItemEntry->GetItemInstance();
	}

	return nullptr;
}

void AInventoryBase::OnRemoveItem(FInventoryItemEntry& ItemEntry)
{
	TScriptInterface<IInventoryItemInstanceInterface> Instance = ItemEntry.GetItemInstance();
	if (IsValid(Instance.GetObject()))
	{
		Instance->OnRemovedFromInventory(ItemEntry, InventoryHandle);
	}

	// Broadcast the change event
	NotifyItemRemoved(ItemEntry, ItemEntry.LastObservedStackCount, 0);
}

void AInventoryBase::OnGiveItem(FInventoryItemEntry& ItemEntry)
{
	if (!IsValid(ItemEntry.ItemDefinition))
	{
		return;
	}

	TScriptInterface<IInventoryItemInstanceInterface> Instance = ItemEntry.GetItemInstance();
	if (Instance == nullptr)
	{
		// Create a new instance for this item entry in case we have a non-replicated one
		const UObject* CDO =ItemEntry.ItemDefinition->GetItemInstanceClass()->GetDefaultObject();
		const IInventoryItemInstanceInterface* InstanceInterface = Cast<IInventoryItemInstanceInterface>(CDO);
		
		if (InstanceInterface && ShouldCreateNewInstanceOfItem(ItemEntry) && !InstanceInterface->GetIsReplicated())
		{
			Instance = CreateNewInstanceOfItem(ItemEntry);

			if (ensure(Instance))
			{
				Instance->OnAddedToInventory(ItemEntry, InventoryHandle);
			}
		}
	}
	
	// Broadcast the change event
	NotifyItemAdded(ItemEntry, ItemEntry.LastObservedStackCount,
		ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize));
}

TArray<TScriptInterface<IInventoryItemInstanceInterface>> AInventoryBase::GetAllItemInstancesAsInterfaces() const
{
	TArray<TScriptInterface<IInventoryItemInstanceInterface>> Result;
	Result.Reserve(GetAllItemInstances().Num());


	TArray<UObject*> Instances = GetAllItemInstances();
	for (UObject* Instance : Instances)
	{
		TScriptInterface<IInventoryItemInstanceInterface> InstanceAsInterface = Instance;
		if (InstanceAsInterface.GetInterface() != nullptr)
		{
			Result.Add(InstanceAsInterface);
		}
	}


	return Result;
}

void AInventoryBase::EvaluateItemEntry(FInventoryOp_ItemAction::FParams& Params)
{
	if (!ensure(Params.ItemEntry->ItemDefinition))
	{
		return;
	}

	if (!ensure(Params.ItemEntry->GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize)) ==
		Params.Delta)
	{
		Params.Delta = Params.ItemEntry->GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
	}
	
	for (const FItemComponentData* ItemData : Params.ItemEntry->ItemDefinition->GetDataList())
	{
		ItemData->EvaluateItemEntry(Params);
	}
}

void AInventoryBase::NativeGiveItem(const TInventoryOpRef<FInventoryOp_ItemAction> Op)
{
	using namespace Itemization::Tags;
	
	FInventoryOp_ItemAction::FParams& Params = Op->Params;
	FInventoryOp_ItemAction::FResult& Result = Op->Result;

	// We assume that we couldn't add anything yet, as we will do this later underneath
	Result.Excess = Params.Delta;

	FInventoryItemHandle LastRelevantHandle = FInventoryItemHandle::InvalidHandle;

	// Clamping to make sure we always have at least 1 max stack size
	const int32 MaxStackSize = FMath::Max(Params.ItemEntry->GetStatValue(TAG_ItemStat_MaxStackSize), 1);

	// Try to find existing stacks first and fill them up
	// Only after that, we will create a new stack if we can
	if (MaxStackSize > 1)
	{
		// Iterate over all items in the inventory and try to find a stack that we can merge with
		for (FInventoryItemEntry& FoundEntry : InventoryList)
		{
			// Skip items that are not of the same type
			if (FoundEntry.ItemDefinition != Params.ItemEntry->ItemDefinition)
			{
				continue;
			}

			const int32 OldStackSize = FoundEntry.GetStatValue(TAG_ItemStat_CurrentStackSize);
			if (CanMergeItems(*Params.ItemEntry, FoundEntry))
			{
				// Merge the items
				int32 MergeExcess;
				MergeItems(*Params.ItemEntry, FoundEntry, MergeExcess);

				// Update the excess count
				Result.Excess = FMath::Max(MergeExcess, 0);

				// Update the last relevant handle
				LastRelevantHandle = FoundEntry.ItemHandle;

				// Broadcast the change
				NotifyItemChanged(
					FoundEntry,
					OldStackSize,
					FoundEntry.GetStatValue(TAG_ItemStat_CurrentStackSize));

				// Mark the item dirty for replication
				MarkItemEntryDirty(FoundEntry, true);
			}
		}
	}

	// If we still have excess items, we can create a new stack
	while (Result.Excess > 0)
	{
		// Sometimes we may not be able to create a new stack.
		// E.g., if the item is restricted to a single stack and we already have one
		if (!CanCreateNewStack(Params))
		{
			break;
		}

		// Subtract the new stack delta from the excess
		const int32 NewStackDelta = FMath::Min(Result.Excess, MaxStackSize);
		Result.Excess -= NewStackDelta;

		// Create a copy of the item entry and update its stack size
		FInventoryItemEntry NewEntryCopy = *Params.ItemEntry;
		NewEntryCopy.SetStatValue(TAG_ItemStat_CurrentStackSize, NewStackDelta);

		// Now add it to the inventory and generate a new uid
		FInventoryItemEntry& NewEntry = InventoryList.Items.Add_GetRef(NewEntryCopy);
		NewEntry.ItemHandle.GenerateNewUID();
		LastRelevantHandle = NewEntry.ItemHandle;

		// Create a new instance server-side if we need to
		if (ShouldCreateNewInstanceOfItem(NewEntry))
		{
			CreateNewInstanceOfItem(NewEntry);
		}

		// Initialize the just give item entry
		OnGiveItem(NewEntry);

		// Mark the item dirty for replication
		MarkItemEntryDirty(NewEntry, true);
	}

	// Make sure the excess doesn't go below 0
	Result.Excess = FMath::Max(Result.Excess, 0);
	Result.ItemHandle = LastRelevantHandle;
}

bool AInventoryBase::NativeRemoveItem(TInventoryOpRef<FInventoryOp_RemoveItem> Op, bool bRecursive)
{
	int32 NumToRemove = Op->Params.Delta != INDEX_NONE ? FMath::Abs(Op->Params.Delta) : TNumericLimits<int32>::Max();

	//@TODO: Check pending adds first

	bool bRemovedAny = false;
	for (auto It = InventoryList.CreateIterator(); It; ++It)
	{
		// Do we still have items to remove?
		if (NumToRemove <= 0)
		{
			break;
		}

		// Check the filter
		FInventoryItemEntry& ItemEntry = *It;
		if (!Op->Params.FilterFunc(ItemEntry))
		{
			continue;
		}

		using namespace Itemization::Tags;

		const int32 CurrentStackSize = ItemEntry.GetStatValue(TAG_ItemStat_CurrentStackSize);
		const int32 Delta = FMath::Min(NumToRemove, CurrentStackSize);

		// Update the new stack sizes
		ItemEntry.SetStatValue(TAG_ItemStat_CurrentStackSize, CurrentStackSize - Delta);
		NumToRemove -= Delta;

		bRemovedAny = true;

#if ENABLE_DRAW_DEBUG
		ITEMIZATION_LOG("Removing item '%s' [%s]\tSize: %d/%d\tSource: %s",
			*GetNameSafe(ItemEntry.ItemDefinition),
			*ItemEntry.ItemHandle.ToString(),
			CurrentStackSize - Delta,
			ItemEntry.GetStatValue(TAG_ItemStat_MaxStackSize),
			*GetNameSafe(ItemEntry.SourceObject.Get()));
#endif

		// Mark dirty for replication
		MarkItemEntryDirty(ItemEntry, true);

		// If the stack size is now empty, remove the item from the inventory
		if (ItemEntry.GetStatValue(TAG_ItemStat_CurrentStackSize) <= 0)
		{
			//@TODO: Ask item components first if we can remove the item
			bool bCanRemoveItem = true;

			if (bCanRemoveItem)
			{
				//@TODO: Perform a scope lock to make sure no other operations are running

				OnRemoveItem(ItemEntry);
				It.RemoveCurrent();

				// Mark list dirty to replicate the entire thing
				InventoryList.MarkArrayDirty();
			}
		}
	}
	
	return bRemovedAny;
}

bool AInventoryBase::CanMergeItems(const FInventoryItemEntry& ThisEntry, const FInventoryItemEntry& OtherEntry) const
{
	// Can't merge items if they're a different type
	if (ThisEntry.ItemDefinition != OtherEntry.ItemDefinition)
	{
		return false;
	}

	// Iterate over the data list and see if any of them are incompatible or restricted
	for (const FItemComponentData* ItemData : OtherEntry.ItemDefinition->GetDataList())
	{
		if (!ensure(ItemData))
		{
			continue;
		}

		if (!ItemData->CanMergeItems(ThisEntry, OtherEntry))
		{
			return false;
		}
	}
	return true;
}

void AInventoryBase::MergeItems(const FInventoryItemEntry& ThisEntry, FInventoryItemEntry& OtherEntry, int32& OutExcess) const
{
	// Gather max stack size
	const int32 MaxStackSize = ThisEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_MaxStackSize);

	// Calculate the excess number of items that couldn't be added to the base stack
	const int32 ThisStackSize = ThisEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
	const int32 OtherStackSize = OtherEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
	OutExcess = ThisStackSize + OtherStackSize - MaxStackSize;

	OtherEntry.SetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize,
		FMath::Min(MaxStackSize, ThisStackSize + OtherStackSize));
}

bool AInventoryBase::CanCreateNewStack(const FInventoryOp_ItemAction::FParams& Params)
{
	if (!IsValid(Params.ItemEntry->ItemDefinition))
	{
		return false;
	}

	// If we only allow a single stack of the item, only return true if we don't already have one
	if (FItemComponentData_Traits::HasTrait(Params.ItemEntry->ItemDefinition,
		UItemizationCoreSettings::Get()->SingleStackTag))
	{
		return InventoryList.Items.Contains(Params.ItemEntry->ItemDefinition) == false;
	}
	
	return true;
}

bool AInventoryBase::ShouldCreateNewInstanceOfItem(const FInventoryItemEntry& ItemEntry) const
{
	return ItemEntry.ItemDefinition->WantsItemInstance();
}

TScriptInterface<IInventoryItemInstanceInterface> AInventoryBase::CreateNewInstanceOfItem(FInventoryItemEntry& ItemEntry)
{
	checkf(ItemEntry.GetItemInstance() == nullptr, TEXT("Item instance already exists for item [%s]!"),
		*ItemEntry.GetDebugString());

	const UItemDefinitionBase* Definition = ItemEntry.ItemDefinition;
	check(Definition);

	// Find the item instance, fallback to the default one in case none was specified
	const UClass* InstanceClass = ItemEntry.ItemDefinition->GetItemInstanceClass().LoadSynchronous();
	if (InstanceClass == nullptr)
	{
		InstanceClass = UInventoryItemInstance::StaticClass();
	}

	// Create the new instance
	UObject* NewInstance = NewObject<UObject>(this, InstanceClass);
	check(NewInstance);

	IInventoryItemInstanceInterface* InstanceInterface = Cast<IInventoryItemInstanceInterface>(NewInstance);
	checkf(InstanceInterface, TEXT("Item instance class [%s] does not implement IInventoryItemInstanceInterface!"),
		*GetNameSafe(InstanceClass));

	// Add it to our instances-list so that it doesn't get garbage collected
	if (InstanceInterface->GetIsReplicated())
	{
		AddReplicatedItemInstance(NewInstance);
		ItemEntry.SetReplicatedItemInstance(NewInstance);
	}
	else
	{
		ItemEntry.SetNonReplicatedItemInstance(NewInstance);
	}

	ITEMIZATION_ERROR("Created a new item instance %s", *GetNameSafe(NewInstance));

	return NewInstance;
}

void AInventoryBase::NotifyItemAdded(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount)
{
}

void AInventoryBase::NotifyItemRemoved(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount)
{
}

void AInventoryBase::NotifyItemChanged(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount)
{
}

void AInventoryBase::OnRep_InventoryList()
{
	for (FInventoryItemEntry& Entry : InventoryList)
	{
		if (!IsValid(Entry.GetItemInstance().GetObject()))
		{
			// Queue up another call to make sure we get the item instance
			GetWorld()->GetTimerManager().SetTimer(OnRep_InventoryListTimerHandle, this, &ThisClass::OnRep_InventoryList, 0.5f);
			return;
		}
	}
}

void AInventoryBase::AddReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance)
{
	TArray<TObjectPtr<UObject>>& ReplicatedInstances = GetAllItemInstances_Mutable();
	if (ReplicatedInstances.Find(ItemInstance.GetObject()) == INDEX_NONE)
	{
		// Store it in the replicated list to avoid GC
		ReplicatedInstances.Add(ItemInstance.GetObject());

		// Add it to the replicated sub object list if we're replicating
		if (IsUsingRegisteredSubObjectList())
		{
			AddReplicatedSubObject(ItemInstance.GetObject(), COND_None);
		}
	}
}

void AInventoryBase::RemoveReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance)
{
	const bool bWasRemoved = GetAllItemInstances_Mutable().RemoveSingle(ItemInstance.GetObject()) > 0;

	// Remove it from the replicated sub object list if we're replicating
	if (bWasRemoved && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance.GetObject());
	}
}

void AInventoryBase::MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange)
{
	if (Owner->HasAuthority())
	{
		if (ItemEntry.GetItemInstance() == nullptr || bWasAddOrChange)
		{
			InventoryList.MarkItemDirty(ItemEntry);
		}
		else
		{
			// If the instance is invalid, and we're not adding or changing, we need to replicate the entire array
			InventoryList.MarkArrayDirty();
		}
	}
	else
	{
		// Client-side, mark the entire array dirty so it will be replicated
		InventoryList.MarkArrayDirty();
	}
}

void AInventoryBase::FetchOpValidness()
{
	TArray<uint32> RemoveIndices;
	for (const auto& OpPtr : OpCache.GetPendingOperations())
	{
		if (!OpPtr->bPendingRemoval)
		{
			OpPtr->bPendingRemoval = true;
			continue;
		}

		RemoveIndices.AddUnique(OpPtr->OpIndex);

		ITEMIZATION_LOG("Adding operation %s [%u] to removal list, it has been alive for approx. %.2f seconds.",
			*OpPtr->OpName, OpPtr->OpIndex, InventoryCVars::CVarInventoryNetPriority->GetFloat());
	}

	for (const uint32& RemoveIndex : RemoveIndices)
	{
		OpCache.RemoveOp(RemoveIndex);
	}
}
