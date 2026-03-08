// Author: Tom Werner (MajorT), 2025


#include "Inventory/InventoryBase.h"

#include "ItemizationCoreLogChannels.h"
#include "ItemizationCoreStats.h"
#include "ItemizationCoreSettings.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/ActorChannel.h"
#include "Inventory/InventoryConfigAsset.h"
#include "Inventory/Operations/InventoryOp.h"
#include "Inventory/Operations/InventoryOp_PlaceItemInSlot.h"
#include "ItemizationCore/Public/Inventory/Operations/InventoryOp_GiveItem.h"
#include "Items/InventoryItemInstance.h"
#include "Items/ItemDefinitionBase.h"
#include "Items/Data/ItemComponentData_DisallowInventorySlot.h"
#include "Items/Data/ItemComponentData_PickupBase.h"
#include "Items/Data/ItemComponentData_Traits.h"
#include "Net/UnrealNetwork.h"
#include "Pickup/ItemPickupInterface.h"
#include "Pickup/PickupCreationData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryBase)


namespace InventoryCVars
{
	static float MaxInventoryOpLifetime = 10.f;
	static FAutoConsoleVariableRef CVarMaxInventoryOperationLifetime(
		TEXT("Inventory.MaxInventoryOperationLifetime"),
		MaxInventoryOpLifetime,
		TEXT("How long a single inventory operation should be kept alive before it will get removed from the cache."));

	static bool bReplicateItemsToSimulatedProxies = false;
	static FAutoConsoleVariableRef CVarReplicateItemsToSimulatedProxies(
		TEXT("Inventory.ReplicateItemsToSimulatedProxies"),
		bReplicateItemsToSimulatedProxies,
		TEXT("When false, Item Instances replicate to AutonomousProxies only, not SimulatedProxies"));
}

AInventoryBase::AInventoryBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
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

TInventoryOpPtr<FInventoryOp_GiveItem> AInventoryBase::GiveItem(FInventoryOp_GiveItem::FParams&& Params)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_GiveItem)

	if (!Params.AreParamsValid())
	{
		ITEMIZATION_WARN("Called with invalid Params [%s].", *Params.GetDebugString())
		return nullptr;
	}

	if (!HasAuthority())
	{
		ITEMIZATION_WARN("Called on an actor with no authority. Not allowed!")
		return nullptr;
	}

	// Create the item entry and notify the item data about it
	if (!Params.ItemEntry)
	{
		if (!Params.ItemDefinition.IsValid())
		{
			ITEMIZATION_WARN("Attempted to give an item without providing an item entry or an item definition. At least one is required.")
			return nullptr;
		}

		// Create a new item entry and store it in the params
		FInventoryItemEntry NewItemEntry = CreateItemEntry(Params.ItemDefinition.Get(), Params.NumItems, Params.SourceObject.Get());
		Params.ItemEntry = &NewItemEntry;
	}


	ITEMIZATION_LOG("Giving item (%s) with count (%d) and source (%s) to inventory (%s)",
		*GetNameSafe(Params.ItemEntry->ItemDefinition),
		Params.NumItems,
		*GetNameSafe(Params.SourceObject.Get()),
		*GetName())

	// Create the operation & process the operation
	TInventoryOpRef<FInventoryOp_GiveItem> NewOp = MakeSharedOp<FInventoryOp_GiveItem>(Params);
	ProcessGiveItemOperation(NewOp);

	return NewOp;
}

TInventoryOpPtr<FInventoryOp_RemoveItem> AInventoryBase::RemoveItem(FInventoryOp_RemoveItem::FParams&& Params)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_RemoveItem)

	if (!Params.AreParamsValid())
	{
		ITEMIZATION_WARN("Called with invalid Params [%s].", *Params.GetDebugString());
		return nullptr;
	}

	if (!HasAuthority())
	{
		ITEMIZATION_WARN("Called on an actor with no authority. Not allowed!")
		return nullptr;
	}

	ITEMIZATION_LOG("Removing item (%s), num remove (%d), from inventory (%s)",
		*Params.GetFilterDebugString(),
		Params.NumRemove,
		*GetName())

	// Create the operation & process the operation
	TInventoryOpRef<FInventoryOp_RemoveItem> NewOp = MakeSharedOp<FInventoryOp_RemoveItem>(Params);
	ProcessRemoveItemOperation(NewOp);

	return NewOp;
}

AActor* AInventoryBase::DropItem(FInventoryItemEntry* ItemEntry, int32 NumToDrop)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_DropItem)

	if (ItemEntry == nullptr)
	{
		ITEMIZATION_WARN("Was passed an invalid item entry.")
		return nullptr;
	}

	FInventoryOp_RemoveItem::FParams Params;
	Params.ItemId = ItemEntry->GetItemId();
	Params.NumRemove = NumToDrop;

	return DropItemImpl(Params, *ItemEntry);
}


AActor* AInventoryBase::DropItemImpl(
	FInventoryOp_RemoveItem::FParams& DropParams,
	const FInventoryItemEntry& ItemEntry)
{

	// Take a snapshot of the item entry
	FInventoryItemEntry ItemEntryCopy = ItemEntry;
	// We also copy the instance as we may need it in MakePickupCreationData()
	// It's safe as the default copy assignment operator doesnt copy them with
	ItemEntryCopy.NonReplicatedInstance = ItemEntry.NonReplicatedInstance;
	ItemEntryCopy.ReplicatedInstance = ItemEntry.ReplicatedInstance;

	// Now we can remove
	auto RemoveOp = RemoveItem(MoveTemp(DropParams));

	if (!RemoveOp->Result.bRemovedAny)
	{
		ITEMIZATION_WARN("no stacks were removed for item %s", *ItemEntry.GetDebugString())
		return nullptr;
	}

	// Construct the pickup data and set it's stack count to the amount of items that got removed
	ItemEntryCopy.SetStackSizeNoDirty(RemoveOp->Result.NumRemoved);

	FPickupCreationData CreationData = MakePickupCreationData(ItemEntryCopy);
	if (!CreationData.IsValid())
	{
		ITEMIZATION_WARN("couldn't make the pickup creation data for item %s", *ItemEntryCopy.GetDebugString())
		return nullptr;
	}

	// Spawn the pickup
	return SpawnPickupActor(CreationData);
}

AActor* AInventoryBase::DropItem(TScriptInterface<IInventoryItemInstanceInterface> ItemInstance, int32 NumToDrop)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_DropItem)

	if (ItemInstance == nullptr)
	{
		ITEMIZATION_WARN("Was passed an invalid item instance.")
		return nullptr;
	}

	FInventoryOp_RemoveItem::FParams Params;
	Params.ItemId = ItemInstance->GetItemEntry()->GetItemId();
	Params.NumRemove = NumToDrop;

	return DropItemImpl(Params, *ItemInstance->GetItemEntry());
}

AActor* AInventoryBase::DropItem(const FInventoryItemId& ItemId, int32 NumToDrop)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_DropItem)

	if (!ItemId.IsValid())
	{
		ITEMIZATION_WARN("Was passed an invalid item id.")
		return nullptr;
	}

	FInventoryOp_RemoveItem::FParams Params;
	Params.ItemId = ItemId;
	Params.NumRemove = NumToDrop;

	return DropItemImpl(Params, *FindItemEntryById(ItemId));
}

AActor* AInventoryBase::DropItem(const UItemDefinitionBase* ItemDefinition, int32 NumToDrop)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_DropItem)

	if (ItemDefinition == nullptr)
	{
		ITEMIZATION_WARN("Was passed an invalid item id.")
		return nullptr;
	}

	FInventoryItemEntry* ItemToDrop = FindFirstItemEntryByDefinition(ItemDefinition);
	if (ItemToDrop == nullptr)
	{
		ITEMIZATION_WARN("Was passed a valid item definition, but doesn't have it in the inventory.")
		return nullptr;
	}

	FInventoryOp_RemoveItem::FParams Params;
	Params.ItemDefinition = ItemDefinition;
	Params.NumRemove = NumToDrop;

	return DropItemImpl(Params, *ItemToDrop);
}

bool AInventoryBase::CanAutoCombineStacks(const UItemDefinitionBase* ItemDefinition) const
{
	// Check for the trait tag
	return IsValid(ItemDefinition) && ItemDefinition->HasTrait(UItemizationCoreSettings::Get()->AutoCombineStacks);
}

bool AInventoryBase::CanCombineItems(const FInventoryItemEntry& This, const FInventoryItemEntry& Other) const
{
	// Can't combine items from different type
	if (This.GetItemDefinition() != Other.GetItemDefinition())
	{
		return false;
	}

	// Check if the stack is already full
	if (Other.GetStackSize() >= Other.GetItemDefinition()->GetMaxStackSize())
	{
		return false;
	}

	// Ask both item instances individually
	// It's okay if one doesn't have an item instance yet
	if (This.GetItemInstance())
	{
		if (!This.GetItemInstance()->CanCombineWith(Other))
		{
			return false;
		}
	}

	if (Other.GetItemInstance())
	{
		if (!Other.GetItemInstance()->CanCombineWith(This))
		{
			return false;
		}
	}

	return true;
}

bool AInventoryBase::CanRemoveItem(const FInventoryItemEntry& ItemEntry) const
{
	if (!IsValid(ItemEntry.GetItemDefinition()))
	{
		return true;
	}

	// Check for the trait that allows an empty stack, if present, not allow removing
	return ItemEntry.GetItemDefinition()->HasTrait(UItemizationCoreSettings::Get()->AllowEmptyStackTag) == false;
}

void AInventoryBase::MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange, bool bForceMarkItemDirty)
{
	if (HasAuthority())
	{
		if (bWasAddOrChange || bForceMarkItemDirty)
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
		// Client-side. mark the entire array dirty so it will be replicated
		InventoryList.MarkArrayDirty();
	}
}

TScriptInterface<IInventoryItemInstanceInterface> AInventoryBase::CreateNewItemInstance(FInventoryItemEntry& ItemEntry)
{
	checkf(ItemEntry.GetItemInstance() == nullptr, TEXT("Item entry (%s) already has an instance!"),
		*ItemEntry.GetDebugString())

	const UItemDefinitionBase* ItemDefinition = ItemEntry.GetItemDefinition();
	check(ItemDefinition)

	// Find the item instance class, but fallback to default in case none was specified
	const UClass* Class = ItemDefinition->GetItemInstanceClass().LoadSynchronous();
	if (Class == nullptr)
	{
		Class = UInventoryItemInstance::StaticClass();
	}

	// Create the new instance
	UObject* NewInstance = NewObject<UObject>(this, Class);
	check(NewInstance)

	IInventoryItemInstanceInterface* InstanceAsInterface = Cast<IInventoryItemInstanceInterface>(NewInstance);
	checkf(InstanceAsInterface, TEXT("Item instance of type '%s' does not implement IInventoryItemInstanceInterface!"),
		*Class->GetName())

	// Add it to our instances list so that it doesn't get GCd
	if (InstanceAsInterface->GetIsReplicated())
	{
		AddReplicatedItemInstance(NewInstance);
		ItemEntry.SetReplicatedItemInstance(NewInstance);
	}
	else
	{
		ItemEntry.SetNonReplicatedItemInstance(NewInstance);
	}

	ITEMIZATION_DISPLAY("Created a new item instance (%s) for '%s'",
		*GetNameSafe(NewInstance), *ItemEntry.GetDebugString())

	return NewInstance;
}

TScriptInterface<IInventoryItemInstanceInterface> AInventoryBase::FindItemInstanceById(
	const FInventoryItemId& ItemId) const
{
	return InventoryList.FindItemInstanceById(ItemId);
}

TArray<TScriptInterface<IInventoryItemInstanceInterface>> AInventoryBase::GetAllItemInstances() const
{
	TArray<TScriptInterface<IInventoryItemInstanceInterface>> Result;

	// O(n)
	for (const FInventoryItemEntry& ItemEntry : InventoryList)
	{
		if (ItemEntry.GetItemInstance())
		{
			Result.Add(ItemEntry.GetItemInstance());
		}
	}

	return MoveTemp(Result);
}

FInventoryItemEntry* AInventoryBase::FindItemEntryById(const FInventoryItemId& ItemId) const
{
	return InventoryList.FindItemEntryById(ItemId);
}

FInventoryItemEntry* AInventoryBase::FindFirstItemEntryByDefinition(const UItemDefinitionBase* ItemDefinition) const
{
	return InventoryList.FindFirstItemEntryByDefinition(ItemDefinition);
}

void AInventoryBase::OnGiveItem(FInventoryItemEntry& ItemEntry)
{
	if (!ensure(ItemEntry.IsValid()))
	{
		return;
	}

	// This can also be called due to replication.
	// Therefore, we may need to create a new item instance as the instance might not be replicated
	TScriptInterface<IInventoryItemInstanceInterface> Instance = ItemEntry.GetItemInstance();
	if (Instance == nullptr)
	{
		const UClass* Class = ItemEntry.GetItemDefinition()->GetItemInstanceClass().LoadSynchronous();
		if (Class == nullptr)
		{
			Class = UInventoryItemInstance::StaticClass();
		}

		const UObject* CDO = Class->GetDefaultObject();
		const IInventoryItemInstanceInterface* InterfaceAsCDO = Cast<IInventoryItemInstanceInterface>(CDO);

		// See above, we only create a new one if the instance doesn't replicate
		if (InterfaceAsCDO && ShouldCreateNewItemInstance(ItemEntry) && !InterfaceAsCDO->GetIsReplicated())
		{
			Instance = CreateNewItemInstance(ItemEntry);

			if (ensure(Instance))
			{
				Instance->OnAddedToInventory(ItemEntry, InventoryHandle);
			}
		}
	}

	// Also notify the item data
	for (const FItemComponentData* ItemData : ItemEntry.GetItemDefinition()->GetDataList())
	{
		if (ItemData != nullptr)
		{
			ItemData->OnItemGiven(ItemEntry, InventoryHandle);
		}
	}

	// Broadcast the change event
	NotifyItemAdded(ItemEntry, ItemEntry.GetLastObservedStackSize(), ItemEntry.GetStackSize());
}

void AInventoryBase::OnRemoveItem(FInventoryItemEntry& ItemEntry)
{
	if (!ensure(ItemEntry.IsValid()))
	{
		return;
	}

	// Notify the item instance about its removal
	TScriptInterface<IInventoryItemInstanceInterface> Instance = ItemEntry.GetItemInstance();
	if (Instance != nullptr)
	{
		Instance->OnRemovedFromInventory(ItemEntry, InventoryHandle);
	}

	// Also notify the item data
	for (const FItemComponentData* ItemData : ItemEntry.GetItemDefinition()->GetDataList())
	{
		if (ItemData != nullptr)
		{
			ItemData->OnItemRemoved(ItemEntry, InventoryHandle);
		}
	}

	// Broadcast the remove event
	NotifyItemRemoved(ItemEntry, ItemEntry.GetLastObservedStackSize(), 0);
}

void AInventoryBase::NotifyItemChanged(
	const FInventoryItemEntry& ItemThatChanged,
	const int32& LastCount,
	const int32& NewCount)
{
	OnItemChangedDelegate.Broadcast(ItemThatChanged, LastCount, NewCount);

	ITEMIZATION_LOG("")
}

void AInventoryBase::NotifyItemAdded(
	const FInventoryItemEntry& ItemThatWasAdded,
	const int32& LastCount,
	const int32& NewCount)
{
	OnItemAddedDelegate.Broadcast(ItemThatWasAdded, LastCount, NewCount);

	ITEMIZATION_LOG("")
}

void AInventoryBase::NotifyItemRemoved(
	const FInventoryItemEntry& ItemThatWasRemoved,
	const int32& LastCount,
	const int32& NewCount)
{
	OnItemRemovedDelegate.Broadcast(ItemThatWasRemoved, LastCount, NewCount);

	ITEMIZATION_LOG("")
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

	FDoRepLifetimeParams SharedParms;
	SharedParms.Condition = COND_ReplayOrOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryList, SharedParms)
}

bool AInventoryBase::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (UObject* ItemInstance : GetReplicatedItemInstances_Mutable())
	{
		if (IsValid(ItemInstance))
		{
			WroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
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
		InventoryCVars::MaxInventoryOpLifetime,
		true, 5.f);

	// Register the item instances to the replicated sub-object list
	if (IsUsingRegisteredSubObjectList())
	{
		for (UObject* ItemInstance : GetReplicatedItemInstances_Mutable())
		{
			if (IsValid(ItemInstance))
			{
				ELifetimeCondition NetCondition = COND_ReplayOrOwner;
				if (const IInventoryItemInstanceInterface* InstanceInterface = Cast<IInventoryItemInstanceInterface>(ItemInstance))
				{
					NetCondition = InstanceInterface->GetNetCondition();
				}

				if (NetCondition != COND_None && InventoryCVars::bReplicateItemsToSimulatedProxies)
				{
					NetCondition = COND_None;
				}

				AddReplicatedSubObject(ItemInstance, NetCondition);
			}
		}
	}
}

bool AInventoryBase::CanCreateNewStack(const FInventoryItemEntry& ItemEntry) const
{
	if (!IsValid(ItemEntry.GetItemDefinition()))
	{
		return false;
	}

	// Check for the single stat trait tag
	// Worst case O(n²)
	if (ItemEntry.GetItemDefinition()->HasTrait(UItemizationCoreSettings::Get()->SingleStackTag))
	{
		if (InventoryList.Items.Contains(ItemEntry.GetItemDefinition()))
		{
			ITEMIZATION_WARN("Item can only have a single stack, but the a stack of the same type already exists.")
			return false;
		}
	}

	return true;
}

FInventoryItemEntry AInventoryBase::CreateItemEntry(
	const UItemDefinitionBase* ItemDefinition,
	int32 StackCount,
	UObject* SourceObject) const
{
	if (!ensure(ItemDefinition))
	{
		return FInventoryItemEntry();
	}

	// Create the new entry and inform the item data
	FInventoryItemEntry NewEntry(ItemDefinition, StackCount, SourceObject);

	for (const FItemComponentData* ItemData : ItemDefinition->GetDataList())
	{
		ItemData->PostItemEntryCreated(NewEntry);
	}

	return MoveTemp(NewEntry);
}

void AInventoryBase::ProcessGiveItemOperation(const TInventoryOpRef<FInventoryOp_GiveItem>& GiveOp)
{
	// Cache parms
	FInventoryOp_GiveItem::FParams& Params = GiveOp->Params;
	FInventoryOp_GiveItem::FResult& Result = GiveOp->Result;
	FInventoryItemEntry& ThisItem = *Params.ItemEntry;

	// We grab item definition from entry, as the direct item definition inside the params may not be valid
	const UItemDefinitionBase* ItemDefinition = Params.ItemEntry->GetItemDefinition();

	// We assume that we couldn't add anything yet
	Result.Excess = Params.NumItems;


	FInventoryItemId LastRelevantId = FInventoryItemId::InvalidId;

	// Get Max Stack Size
	const int32 MaxStackSize = ItemDefinition->GetMaxStackSize();

	// Try to find existing stacks first and fill them up
	// Only after that, we will create a new stack if we can
	if (MaxStackSize > 1 && CanAutoCombineStacks(ItemDefinition))
	{
		// Iterate over all items in the inventory and try to find a stack that we can merge with
		for (FInventoryItemEntry& OtherItem : InventoryList)
		{
			// Check if this item was completely combined with others and how has a zero stack size
			if (ThisItem.GetStackSize() <= 0)
			{
				break;
			}

			// Early out on items of a different type
			if (OtherItem.GetItemDefinition() != ItemDefinition)
			{
				continue;
			}

			// Ask whether these items can be combined
			if (!CanCombineItems(ThisItem, OtherItem))
			{
				continue;
			}

			// Combine them
			int32 NumCouldNotCombine;
			CombineItems(ThisItem, OtherItem, NumCouldNotCombine);

			// Update the excess
			Result.Excess = FMath::Max(0, NumCouldNotCombine);

			// Update the last item id
			LastRelevantId = OtherItem.GetItemId();
		}
	}

	// We could have filled up existing stacks above
	// No check if there are any stacks left to be added
	// This will now create a new item entry + instance
	while (Result.Excess > 0)
	{
		int32 CreatedStackSize;
		if (!AttemptCreateNewStack(ThisItem, LastRelevantId, Result.Excess, CreatedStackSize, &Params))
		{
			break;
		}

		// Subtract the stack size from  the excess
		Result.Excess -= CreatedStackSize;
		ITEMIZATION_LOG("We still have %d [just created %d] items left, continue to create new stacks.", Result.Excess, CreatedStackSize)
	}

	// Clamp excess to 0
	Result.Excess = FMath::Max(0, Result.Excess);
	Result.ItemId = LastRelevantId;
}

void AInventoryBase::ProcessRemoveItemOperation(const TInventoryOpRef<FInventoryOp_RemoveItem>& RemoveOp)
{
	// Cache params
	FInventoryOp_RemoveItem::FParams& Params = RemoveOp->Params;
	FInventoryOp_RemoveItem::FResult& Result = RemoveOp->Result;

	// Negative values, means remove all
	int32 NumToRemove = Params.NumRemove <= 0 ? TNumericLimits<int32>::Max() : Params.NumRemove;

	bool bRemovedAny = false;
	TArray<FInventoryItemId> ItemsToRemove;
	for (auto It = InventoryList.CreateIterator(); It; ++It)
	{
		// Early out, in case we got no more to remove
		if (NumToRemove <= 0)
		{
			break;
		}

		FInventoryItemEntry& Other = *It;

		// Check the filter
		if (!MatchesRemoveFilter(Other, Params))
		{
			UE_LOG(LogTemp, Warning, TEXT("DOESNT MATHC FILTER -- ItemEntry.ItemId(%u) != Params.ItemId(%u)"),
			Other.GetItemId().Get(), Params.ItemId.Get())
			continue;
		}

		// Get the new stack size
		const int32 OldStackSize = Other.GetStackSize();
		const int32 ActualNumRemove = FMath::Min(NumToRemove, OldStackSize);

		// Check if the stack would be empty now
		// Remove the entire stack, in case this item can't have an empty stack
		if ((OldStackSize - ActualNumRemove) <= 0 && CanRemoveItem(Other))
		{
			ItemsToRemove.Add(Other.GetItemId());
		}
		else
		{
			// Update the new stack size
			Other.SetStackSize(OldStackSize - ActualNumRemove);

			// Broadcast the change
			NotifyItemChanged(Other, OldStackSize, OldStackSize - ActualNumRemove);
		}

		// Keep track of remove count
		NumToRemove -= ActualNumRemove;
		Result.NumRemoved += ActualNumRemove;
		bRemovedAny = true;

		// We removed at least one item
		// Lets check whether our remove params allow multiple matches
		// If not, we early out to save some iteration overhead
		if (!Params.MightHaveMultipleMatches())
		{
			break;
		}
	}

	// Remove all the items
	for (const FInventoryItemId& IdToRemove : ItemsToRemove)
	{
		InventoryList.RemoveItemFromList(IdToRemove);
	}

	Result.NumMissing = FMath::Max(0, NumToRemove);
	Result.bRemovedAny = bRemovedAny;
}


bool AInventoryBase::MatchesRemoveFilter(
	const FInventoryItemEntry& ItemEntry,
	FInventoryOp_RemoveItem::FParams& Params) const
{
	if (Params.ItemInstance.IsValid())
	{
		return ItemEntry.GetItemInstance() == Params.ItemInstance.Get();
	}

	if (Params.ItemDefinition.IsValid())
	{
		return ItemEntry.GetItemDefinition() == Params.ItemDefinition.Get();
	}

	if (Params.ItemId.IsValid())
	{
		const bool bResult = ItemEntry.GetItemId() == Params.ItemId;
		UE_LOG(LogTemp, Warning, TEXT("RESULT %d -- ItemEntry.ItemId(%u) == Params.ItemId(%u)"),
			bResult, ItemEntry.GetItemId().Get(), Params.ItemId.Get())
		return bResult;
	}

	if (Params.HasValidFilterFunc())
	{
		return Params.FilterFunc(ItemEntry);
	}

	ensureMsgf(false, TEXT("Remove Item operation has invalid filters"));
	return false;
}

void AInventoryBase::CombineItems(
	FInventoryItemEntry& This,
	FInventoryItemEntry& Other,
	int32& OutCouldNotCombine)
{
	//@TODO: We may have to remove This item in case it could be fully combined and is inside this inventory

	// Get max stack size
	const int32 OtherMaxStackSize = Other.GetItemDefinition()->GetMaxStackSize();

	// Calculate the excess number of items that couldn't be combined into the Other stack
	const int32 ThisStackSize = This.GetStackSize();
	const int32 OtherStackSize = Other.GetStackSize();

	// This: 3/6, Other 4/6
	// 3 to be added, but maximum 6-4 = 2 can be added.
	const int32 StacksToBeAdded = FMath::Min(ThisStackSize, OtherMaxStackSize - OtherStackSize);
	const int32 ThisNewStackSize = ThisStackSize - StacksToBeAdded;
	const int32 OtherNewStackSize = OtherStackSize + StacksToBeAdded;

	// Update the size
	Other.SetStackSize(OtherNewStackSize);

	// Check if we need to remove This item
	if (ThisStackSize <= 0)
	{
		//@TODO: Mark pending remove idk ??
	}
	else
	{
		This.SetStackSize(ThisNewStackSize);

		// Broadcast the change
		NotifyItemChanged(This, ThisStackSize, ThisNewStackSize);
	}


	// Broadcast the change
	NotifyItemChanged(Other, OtherStackSize, OtherNewStackSize);

	ITEMIZATION_LOG("CombineItems: Combined '%s' [old: %d -> new: %d] amd '%s' [old: %d -> new: %d]",
		*This.GetDebugString(), ThisStackSize, ThisNewStackSize,
		*Other.GetDebugString(), OtherStackSize, OtherNewStackSize)

	OutCouldNotCombine = FMath::Max(0, ThisStackSize - StacksToBeAdded);
}

bool AInventoryBase::AttemptCreateNewStack(
	FInventoryItemEntry& ItemEntry,
	FInventoryItemId& OutItemId,
	const int32& RemainingStacks,
	int32& OutCreatedStackSize,
	FInventoryOp_AdditiveBase::FAdditiveParamsBase* Params)
{
	// Make sure we have at least one remaining stack
	if (RemainingStacks <= 0)
	{
		return false;
	}

	// Sometimes we may not be able to create a new stack.
	// E.g., if the item is restricted to a single stack and we already have one
	if (!CanCreateNewStack(ItemEntry))
	{
		return false;
	}

	// Subtract the new stack size from the excess
	const int32 NewStackSize = FMath::Min(RemainingStacks, ItemEntry.GetItemDefinition()->GetMaxStackSize());
	OutCreatedStackSize = NewStackSize;

	// Create a copy of the item entry and update its stack size
	FInventoryItemEntry EntryCopy = ItemEntry;
	EntryCopy.SetStackSize(NewStackSize);

	// Add the item to the inventory and generate a new uid
	FInventoryItemEntry& NewItemEntry = InventoryList.AddItemToList(MoveTemp(EntryCopy));
	OutItemId = NewItemEntry.GetItemId();

	ITEMIZATION_DISPLAY("Created new stack for '%s' count %d.",
		*GetNameSafe(ItemEntry.GetItemDefinition()), OutCreatedStackSize)

	return true;
}

void AInventoryBase::AddReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance)
{
	TArray<TObjectPtr<UObject>>& AllInstances = GetReplicatedItemInstances_Mutable();
	if (AllInstances.Find(ItemInstance.GetObject()) == INDEX_NONE)
	{
		// Store it in the list to avoid GC
		AllInstances.Add(ItemInstance.GetObject());

		// Add it to the replicated sub-object list if we're replicating
		if (IsUsingRegisteredSubObjectList())
		{
			ELifetimeCondition NetCondition = ItemInstance->GetNetCondition();
			if (NetCondition != COND_None && InventoryCVars::bReplicateItemsToSimulatedProxies)
			{
				NetCondition = COND_None;
			}

			AddReplicatedSubObject(ItemInstance.GetObject(), NetCondition);
		}
	}
}

void AInventoryBase::RemoveReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance)
{
	const bool bWasRemoved = GetReplicatedItemInstances_Mutable().RemoveSingle(ItemInstance.GetObject()) > 0;

	// Remove from the replicated sub-object list if we're replicating
	if (bWasRemoved && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance.GetObject());
	}
}

FPickupCreationData AInventoryBase::MakePickupCreationData(const FInventoryItemEntry& ItemEntry) const
{
	check(ItemEntry.GetItemDefinition() != nullptr);

	if (const FItemComponentData_PickupBase* PickupItemData =
		ItemEntry.GetItemDefinition()->GetItemData<FItemComponentData_PickupBase>())
	{
		return PickupItemData->GetPickupCreationData(ItemEntry, InventoryHandle);
	}

	return FPickupCreationData();
}

AActor* AInventoryBase::SpawnPickupActor(const FPickupCreationData& PickupCreationData) const
{
	check(PickupCreationData.IsValid());

	UWorld* const World = GetWorld();
	const ENetMode NetMode = World->GetNetMode();
	check(NetMode != NM_Client);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = GetOwner();
	SpawnInfo.Instigator = GetOwner<APawn>();
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnInfo.ObjectFlags |= RF_Transient;

	const FPickupCreationData_Impl* DataImpl = PickupCreationData.Get<FPickupCreationData_Impl>();
	UClass* ClassToUse = DataImpl->OverridePickupActorClass
		? DataImpl->OverridePickupActorClass.Get()
		: PickupActorClass.Get();

	if (!ensureAlwaysMsgf(IsValid(ClassToUse), TEXT("Invalid class for spawning pickup actor, default pickup actor class is not set in the inventory actor.")))
	{
		return nullptr;
	}

	FTransform SpawnTransform = {DataImpl->Rotation, DataImpl->Location};

	// Spawn the pickup
	AActor* Spawned = World->SpawnActor(ClassToUse, &SpawnTransform, SpawnInfo);

	IItemPickupInterface* Pickup = Cast<IItemPickupInterface>(Spawned);
	checkf(Pickup, TEXT("Pickup actor (%s) must inherit the IItemPickupInterface!"), *ClassToUse->GetName())
	Pickup->SetupPickupWithCreationData(PickupCreationData);

	// Let subclasses inject custom data into the pickup actor
	PostSpawnPickupActor(PickupCreationData, Spawned);

	return Spawned;
}


void AInventoryBase::OnRep_InventoryList()
{
}


void AInventoryBase::FetchOpValidness()
{
	TArray<int32> RemoveIndices;
	for (const auto& OpPtr : OpCache.GetPendingOperations())
	{
		if (!OpPtr->bPendingRemoval)
		{
			OpPtr->bPendingRemoval = true;
			continue;
		}

		RemoveIndices.AddUnique(OpPtr->OpIndex);

		ITEMIZATION_VERBOSE("Adding operation %s [%u] to removal list, it has been alive for approx. %.2f seconds.",
			*OpPtr->OpName, OpPtr->OpIndex, InventoryCVars::MaxInventoryOpLifetime);
	}

	for (const uint32& RemoveIndex : RemoveIndices)
	{
		OpCache.RemoveOp(RemoveIndex);
	}
}
