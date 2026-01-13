// Author: Tom Werner (MajorT), 2025


#include "Inventory/InventoryBase.h"

#include "ItemizationCoreLogChannels.h"
#include "ItemizationCoreStats.h"
#include "ItemizationCoreSettings.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Inventory/InventoryConfigAsset.h"
#include "Inventory/Operations/InventoryOp.h"
#include "Inventory/Operations/InventoryOp_PlaceItemInSlot.h"
#include "ItemizationCore/Public/Inventory/Operations/InventoryOp_GiveAction.h"
#include "Items/InventoryItemInstance.h"
#include "Items/ItemDefinitionBase.h"
#include "Items/Data/ItemComponentData_DisallowInventorySlot.h"
#include "Items/Data/ItemComponentData_Traits.h"
#include "Net/UnrealNetwork.h"

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
	, InventoryList(this)
	, InventorySlots(this)
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

	FDoRepLifetimeParams SharedParms;
	SharedParms.Condition = COND_AutonomousOnly;

	DOREPLIFETIME_WITH_PARAMS(ThisClass, InventoryList, SharedParms)
	DOREPLIFETIME_WITH_PARAMS(ThisClass, InventorySlots, SharedParms)
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

TInventoryOpPtr<FInventoryOp_GiveAction> AInventoryBase::GiveItem(
	FInventoryOp_GiveAction::FParams&& Params,
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
		Params.NumGive,
		MaxStackSize,
		*GetNameSafe(Params.ItemEntry->SourceObject.Get()))

	Params.ItemEntry->DebugPrintStats();
#endif

	// Create the operation
	TInventoryOpRef<FInventoryOp_GiveAction> NewOp
		= OpCache.MakeSharedOp<FInventoryOp_GiveAction>(MoveTemp(Params));

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

/*FInventoryItemHandle AInventoryBase::GiveItem(TInventoryOpRef<FInventoryOp_GiveAction> Action)
{
	check(Action->Params.ItemEntry);
	FInventoryOp_GiveAction::FParams& Params = Action->Params;

	//@TODO: If locked, add to pending list

	// Evaluate the item entry
	EvaluateItemEntry(Params);
	const int32 MaxStackSize = Params.ItemEntry->GetStatValue(Itemization::Tags::TAG_ItemStat_MaxStackSize);


	ITEMIZATION_LOG_NET("Giving item [%s] %s\tSize: %d/%d\tSource: %s",
		*Params.ItemEntry->ItemHandle.ToString(),
		*GetNameSafe(Params.ItemEntry->ItemDefinition),
		Params.NumRemove,
		MaxStackSize,
		*GetNameSafe(Params.ItemEntry->SourceObject.Get()));
	Params.ItemEntry->DebugPrintStats();

	return NativeGiveItem(Action);
}*/

FInventoryItemEntry* AInventoryBase::FindItemEntryByHandle(const FInventoryItemHandle& ItemHandle) const
{
	SCOPE_CYCLE_COUNTER(STAT_FindItemEntryByHandle)
	return InventoryList.FindItemEntryByHandle(ItemHandle);
}

FInventoryItemSlot* AInventoryBase::FindItemSlotByHandle(const FInventorySlotHandle& SlotHandle) const
{
	SCOPE_CYCLE_COUNTER(STAT_FindItemSlotByHandle)
	return InventorySlots.FindItemSlotByHandle(SlotHandle);
}

TScriptInterface<IInventoryItemInstanceInterface> AInventoryBase::FindItemInstanceByHandle(const FInventoryItemHandle& ItemHandle) const
{
	SCOPE_CYCLE_COUNTER(STAT_FindItemInstanceByHandle)

	if (const FInventoryItemEntry* ItemEntry = FindItemEntryByHandle(ItemHandle))
	{
		return ItemEntry->GetItemInstance();
	}

	return nullptr;
}

TArray<TScriptInterface<IInventoryItemInstanceInterface>> AInventoryBase::GetItemInstancesInGroup(
	const FGameplayTag& InGroup)
{
	SCOPE_CYCLE_COUNTER(STAT_FindItemInstancesInGroup)

	TArray<TScriptInterface<IInventoryItemInstanceInterface>> Result;
	TArray<FInventoryItemHandle> ItemHandlesInGroup = InventorySlots.FindItemHandlesInGroup(InGroup);

	for (const auto& Handle : ItemHandlesInGroup)
	{
		if (const FInventoryItemEntry* ItemEntry = FindItemEntryByHandle(Handle))
		{
			Result.Add(ItemEntry->GetItemInstance());
		}
	}

	return Result;
}

void AInventoryBase::OnRemoveItem(FInventoryItemEntry& ItemEntry)
{
	// Remove the item instance in case we created one
	TScriptInterface<IInventoryItemInstanceInterface> Instance = ItemEntry.GetItemInstance();
	if (IsValid(Instance.GetObject()))
	{
		Instance->OnRemovedFromInventory(ItemEntry, InventoryHandle);
	}

	// Notify the item data
	if (ItemEntry.ItemDefinition)
	{
		for (const FItemComponentData* Data : ItemEntry.ItemDefinition->GetDataList())
		{
			if (Data != nullptr)
			{
				Data->OnItemRemoved(ItemEntry, InventoryHandle);
			}
		}
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

	// Check if we should create a new item instance
	TScriptInterface<IInventoryItemInstanceInterface> Instance = ItemEntry.GetItemInstance();
	if (Instance == nullptr && ItemEntry.ItemDefinition->WantsItemInstance())
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

	// Notify the item data
	for (const FItemComponentData* Data : ItemEntry.ItemDefinition->GetDataList())
	{
		if (Data != nullptr)
		{
			Data->OnItemGiven(ItemEntry, InventoryHandle);
		}
	}

	// Broadcast the change event
	NotifyItemAdded(ItemEntry, ItemEntry.LastObservedStackCount,
		ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize));
}

FInventoryItemSlot* AInventoryBase::GetNextUnoccupiedItemSlot(const FGameplayTag& InGroupTag) const
{
	return InventorySlots.GetNextUnoccupiedItemSlotInGroup(InGroupTag);
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

void AInventoryBase::InitializeInventorySlots(const UInventoryConfigAsset* InventoryConfig)
{
	if (!ensure(IsValid(InventoryConfig)))
	{
		return;
	}

	// Server can't create slots
	if (!HasAuthority())
	{
		return;
	}

	for (const auto& Config : InventoryConfig->InventoryGroupConfigs)
	{
		if (!ensureMsgf(Config.GroupType.IsValid(), TEXT("Attempted to initialize an inventory with an inventory config asset that has an group config with an invalid group tag!!")))
		{
			continue;
		}

		uint32 SlotIndex = 0;
		for (uint32 RowIdx = 0; RowIdx < Config.NumItemRows; ++RowIdx)
		{
			for (uint32 ColIdx = 0; ColIdx < Config.NumItemColumns; ++ColIdx)
			{
				SlotIndex++;

				FInventorySlotHandle SlotHandle(RowIdx, ColIdx);

				// Create the default item slot and assign its values
				FInventoryItemSlot& NewSlot = InventorySlots.AddDefaulted_GetRef();
				NewSlot.SetGroupTag(Config.GroupType);
				NewSlot.SetSlotHandle(SlotHandle);

				// See if we have slot tags
				if (const FGameplayTagContainer* SlotTags = Config.SlotTagMap.Find(SlotIndex))
				{
					NewSlot.SetSlotTags(*SlotTags);

					ITEMIZATION_LOG("Found slot tags for slot [%s: %s]: %s", *Config.GroupType.ToString(), *SlotHandle.ToString(), *SlotTags->ToString())
				}

				// Mark the item dirty so it replicates
				InventorySlots.MarkItemDirty(NewSlot);
			}
		}
	}
}

void AInventoryBase::EvaluateItemEntry(FInventoryOp_GiveAction::FParams& Params)
{
	if (!ensure(Params.ItemEntry->ItemDefinition))
	{
		return;
	}

	if (!ensure(Params.ItemEntry->GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize) ==
		Params.NumGive))
	{
		Params.NumGive = Params.ItemEntry->GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
	}

	for (const FItemComponentData* ItemData : Params.ItemEntry->ItemDefinition->GetDataList())
	{
		ItemData->EvaluateItemEntry(Params);
	}
}

void AInventoryBase::NativeGiveItem(const TInventoryOpRef<FInventoryOp_GiveAction>& Op)
{
	using namespace Itemization::Tags;

	FInventoryOp_GiveAction::FParams& Params = Op->Params;
	FInventoryOp_GiveAction::FResult& Result = Op->Result;

	const UItemDefinitionBase* ItemDefinition = Params.ItemEntry->GetItemDefinition();

	// We assume that we couldn't add anything yet, as we will do this later underneath
	Result.Excess = Params.NumGive;

	FInventoryItemHandle LastRelevantHandle = FInventoryItemHandle::InvalidHandle;

	// Clamping to make sure we always have at least 1 max stack size
	const int32 MaxStackSize = FMath::Max(Params.ItemEntry->GetStatValue(TAG_ItemStat_MaxStackSize), 1);

	// Try to find existing stacks first and fill them up
	// Only after that, we will create a new stack if we can
	if (MaxStackSize > 1 && CanAutoCombineStacks(ItemDefinition))
	{
		// Iterate over all items in the inventory and try to find a stack that we can merge with
		for (FInventoryItemEntry& FoundEntry : InventoryList)
		{
			// Skip items that are not of the same type
			if (FoundEntry.ItemDefinition != ItemDefinition)
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
				MarkInventoryDirty(InventoryList, FoundEntry, true, !FoundEntry.GetItemInstance());
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

		// Try to find a slot that is still unoccupied
		FInventoryItemSlot* SlotToUse = GetNextUnoccupiedItemSlot(Params.GroupTag);
		if (SlotToUse == nullptr)
		{
			// No unoccupied slot was found, so we can't add the item
			break;
		}

		// Check if the item can actually be placed in the found slot
		// Maybe there is an item data that restricts it?
		if (!CanPlaceItemInSlot(ItemDefinition, *SlotToUse))
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

		// Place the item inside the slot
		PlaceItemInSlot(NewEntry, *SlotToUse);

		// Initialize the just give item entry
		OnGiveItem(NewEntry);

		// Mark the item dirty for replication
		MarkInventoryDirty(InventoryList, NewEntry, true, !NewEntry.GetItemInstance());
	}

	// Make sure the excess doesn't go below 0
	Result.Excess = FMath::Max(Result.Excess, 0);
	Result.ItemHandle = LastRelevantHandle;
}

bool AInventoryBase::NativeRemoveItem(const TInventoryOpRef<FInventoryOp_RemoveItem>& Op, bool bRecursive)
{
	int32 NumToRemove = Op->Params.NumRemove != INDEX_NONE ? FMath::Abs(Op->Params.NumRemove) : TNumericLimits<int32>::Max();

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
		MarkInventoryDirty(InventoryList, ItemEntry, true, !ItemEntry.GetItemInstance());

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

void AInventoryBase::NativePlaceItemInSlot(const TInventoryOpRef<FInventoryOp_PlaceItemInSlot>& Op)
{
	FInventoryOp_PlaceItemInSlot::FParams& Params = Op->Params;
	FInventoryOp_PlaceItemInSlot::FResult& Result = Op->Result;

	// Default to false
	Result.bSuccess = false;

	// Make sure we got valid params passed
	if (Params.ItemEntry == nullptr)
	{
		return;
	}

	// Find the item slot we want to place in
	FInventoryItemSlot* TargetSlot = Params.ResolveItemSlot(this);
	if (TargetSlot == nullptr)
	{
		return;
	}


	// Assign the slot
	TargetSlot->SetItemHandle(Params.ItemEntry->GetItemHandle());
	Result.bSuccess = true;

	// Mark dirty for replication
	MarkInventoryDirty(InventorySlots, *Params.ItemEntry, true);
}

TInventoryOpPtr<FInventoryOp_PlaceItemInSlot> AInventoryBase::PlaceItemInSlot(FInventoryItemEntry& ItemEntry, FInventoryItemSlot& Slot)
{
	checkf(Slot.IsUnoccupied(), TEXT("You can't place an item in a slot that is already occupied!!"))
	checkf(ItemEntry.GetItemHandle().IsValid(), TEXT("You can't place an item with an invalid handle (%s) inside a slot!!"), *Slot.GetItemHandle().ToString())

	// Build the place item in slot params
	FInventoryOp_PlaceItemInSlot::FParams Params;
	Params.GroupTag = Slot.GetGroupTag();
	Params.ItemEntry = &ItemEntry;
	Params.TargetSlot = &Slot;

	// Create the operation
	TInventoryOpRef<FInventoryOp_PlaceItemInSlot> NewOp
		= OpCache.MakeSharedOp<FInventoryOp_PlaceItemInSlot>(MoveTemp(Params));

	// Perform the op
	NativePlaceItemInSlot(NewOp);
	return NewOp;
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

bool AInventoryBase::CanCreateNewStack(const FInventoryOp_GiveAction::FParams& Params) const
{
	if (!IsValid(Params.ItemEntry->GetItemDefinition()))
	{
		return false;
	}

	// If we only allow a single stack of the item, only return true if we don't already have one
	if (Params.ItemEntry->GetItemDefinition()->HasTrait(UItemizationCoreSettings::Get()->SingleStackTag))
	{
		if (InventoryList.Items.Contains(Params.ItemEntry->ItemDefinition))
		{
			return false;
		}
	}

	return true;
}

bool AInventoryBase::CanAutoCombineStacks(const UItemDefinitionBase* ItemDefinition) const
{
	if (!IsValid(ItemDefinition))
	{
		return false;
	}

	// Check for the trait tag
	return ItemDefinition->HasTrait(UItemizationCoreSettings::Get()->AutoCombineStacks);
}

bool AInventoryBase::CanPlaceItemInSlot(
	const UItemDefinitionBase* ItemDefinition,
	const FInventoryItemSlot& Slot) const
{
	if (!IsValid(ItemDefinition))
	{
		return false;
	}

	if (const FItemComponentData_DisallowInventorySlot* DisallowSlotData =
		ItemDefinition->GetItemData<FItemComponentData_DisallowInventorySlot>())
	{
		if (Slot.HasAnySlotTags(DisallowSlotData->DisallowedSlotTags))
		{
			return false;
		}
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

void AInventoryBase::OnRep_InventorySlots()
{
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

/*void AInventoryBase::MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange)
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
}*/

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

		ITEMIZATION_LOG("Adding operation %s [%u] to removal list, it has been alive for approx. %.2f seconds.",
			*OpPtr->OpName, OpPtr->OpIndex, InventoryCVars::CVarInventoryNetPriority->GetFloat());
	}

	for (const uint32& RemoveIndex : RemoveIndices)
	{
		OpCache.RemoveOp(RemoveIndex);
	}
}
