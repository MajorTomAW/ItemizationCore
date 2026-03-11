// Author: Tom Werner (dc: majort), 2026


#include "InventoryBase.h"

#include "AudioMixerBlueprintLibrary.h"
#include "ItemizationCoreLogChannels.h"
#include "ItemizationCoreSettings.h"
#include "ItemizationCoreStats.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Config/InventoryConfig.h"
#include "Engine/ActorChannel.h"
#include "Items/ItemDefinitionBase.h"
#include "Items/Data/ItemComponentData_DisallowInventorySlot.h"
#include "Items/Data/ItemComponentData_PickupBase.h"
#include "Items/Data/ItemComponentData_Traits.h"
#include "Net/UnrealNetwork.h"
#include "Pickup/IItemPickupInterface.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryBase)

const int32 AInventoryBase::REMOVE_ENTIRE_STACK = INDEX_NONE;

AInventoryBase::AInventoryBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
	, SlotList(this)
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

void AInventoryBase::InitializeInventorySlots(const UInventoryConfig* InConfig)
{
	if (!ensure(IsValid(InConfig)))
	{
		return;
	}

	// Clients can't create slots
	if (!HasAuthority())
	{
		return;
	}

	ITEMIZATION_LOG(Log, "Initializing inventory slots using '%s'", *InConfig->GetName())

	InventoryConfigAsset = InConfig;

	for (const FInventoryGroupConfig& GroupConfig : InConfig->InventoryGroupConfigs)
	{
		if (!ensureMsgf(GroupConfig.GroupType.IsValid(), TEXT("Attempted to initialize an inventory with an inventory config asset that has an group config with an invalid group tag!!")))
		{
			continue;
		}

		uint32 SlotIndex = 0;
		for (uint32 RowIdx = 0; RowIdx < GroupConfig.NumItemRows; ++RowIdx)
		{
			for (uint32 ColIdx = 0; ColIdx < GroupConfig.NumItemColumns; ++ColIdx)
			{
				FInventorySlotId SlotId(RowIdx, ColIdx);

				// Create the default item slot
				FInventoryItemSlot& NewSlot = SlotList.AddSlotToList_Defaulted(GroupConfig.GroupType);
				NewSlot.SetGroupTag(GroupConfig.GroupType);
				NewSlot.SetSlotId(SlotId);

				ITEMIZATION_LOG(VeryVerbose, "Added item slot [%s]", *SlotId.ToString())

				// See if we have slot tags
				if (const FGameplayTagContainer* SlotTags = GroupConfig.SlotTagMap.Find(SlotIndex))
				{
					NewSlot.SetSlotTags(*SlotTags);

					ITEMIZATION_LOG(VeryVerbose, "Found slot tags for slot [%s: %s]: %s", *GroupConfig.GroupType.ToString(), *SlotId.ToString(), *SlotTags->ToString())
				}

				// Mark the slot dirty for replication
				SlotList.MarkItemDirty(NewSlot);
				SlotIndex++;
			}
		}
	}
}

UItemInstanceBase* AInventoryBase::GiveItem(
	const UItemDefinitionBase* ItemDefinition,
	int32 NumToGive,
	UObject* SourceObject,
	FGameplayTag PreferredGroup,
	int32& OutNumCouldNotGive)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_GiveItem)

	OutNumCouldNotGive = NumToGive;
	FInventoryItemEntry ThisItem = CreateNewItemEntry(ItemDefinition, NumToGive, SourceObject);

	if (!HasAuthority())
	{
		UE_LOG(LogItemization, Warning, NET_TEXT(this, "Attempted to GiveItem on a non authoritative connect. Not allowed!"))
		return nullptr;
	}

	if (!IsValid(ItemDefinition))
	{
		ITEMIZATION_LOG(Warning, "Attempted to GiveItem with an invalid ItemDefinition.")
		return nullptr;
	}

	UItemInstanceBase* LastRelevantInstance = nullptr;

	// Get max stack size
	const int32 MaxStackSize = ItemDefinition->GetMaxStackSize();

	// Try to find existing stacks first and fill them up
	// When done, we will use the remaining NumToGive to create new stacks
	if (MaxStackSize > 1 && CanAutoCombineStacks(ItemDefinition))
	{
		// Iterate overall items in the inventory and try to find a stack that we can combine with
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

			// Update the last item
			OutNumCouldNotGive = FMath::Max(0, NumCouldNotCombine);
			LastRelevantInstance = OtherItem.GetItemInstance();
		}
	}

	// We could have filled up existing stacks above
	// No check if there are any stacks left to be added
	// This will now create a new item entry + instance
	while (OutNumCouldNotGive > 0)
	{
		int32 CreatedStackSize;
		FInventoryItemId NewItemId;
		UItemInstanceBase* NewItem = AttemptCreateNewStack(ThisItem, NewItemId, PreferredGroup, OutNumCouldNotGive, CreatedStackSize);
		if (!IsValid(NewItem))
		{
			break;
		}

		// Subtract stack size from excess
		OutNumCouldNotGive -= CreatedStackSize;
		LastRelevantInstance = NewItem;
	}

	// Make sure we don't go into negative
	OutNumCouldNotGive = FMath::Max(0, OutNumCouldNotGive);
	return LastRelevantInstance;
}

int32 AInventoryBase::RemoveItem(const UItemInstanceBase* Item, int32 NumToRemove)
{
	return RemoveItemById(Item->GetItemId(), NumToRemove);
}

int32 AInventoryBase::RemoveItemById(const FInventoryItemId& ItemId, int32 NumToRemove)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_RemoveItem)

	int32 NumRemoved = FMath::Max(NumToRemove, 0);

	if (!HasAuthority())
	{
		UE_LOG(LogItemization, Warning, NET_TEXT(this, "Attempted to RemoveItem on a non authoritative connect. Not allowed!"))
		return NumRemoved;
	}

	if (!ItemId.IsValid())
	{
		ITEMIZATION_LOG(Warning, "Attempted to RemoveItem with an invalid Item.")
		return NumRemoved;
	}

	for (int32 Idx = 0; Idx < ItemPendingAdds.Num(); Idx++)
	{
		if (ItemPendingAdds[Idx].GetItemId() == ItemId)
		{
			ItemPendingAdds.RemoveAtSwap(Idx, EAllowShrinking::No);
			return ItemPendingAdds[Idx].GetStackSize();
		}
	}

	FInventoryItemId ItemToRemove;
	for (auto It = InventoryList.CreateIterator(); It; ++It)
	{
		// Early out in case we got no more stacks to remove
		if (NumToRemove <= 0 && NumToRemove != REMOVE_ENTIRE_STACK)
		{
			break;
		}

		FInventoryItemEntry& ItemEntry = *It;
		if (ItemEntry.GetItemId() != ItemId)
		{
			continue;
		}

		const int32 OldStackSize = ItemEntry.GetStackSize();
		const int32 EffectiveNumToRemove = (NumToRemove == REMOVE_ENTIRE_STACK)
			? OldStackSize // remove entire stack
			: FMath::Min(NumToRemove, OldStackSize);

		// Check if the item would have an empty stack after removal
		if ((OldStackSize - EffectiveNumToRemove) <= 0 && CanRemoveItem(ItemEntry))
		{
			// Mark to remove
			ItemToRemove = ItemEntry.GetItemId();
		}
		else
		{
			// Just update the stack size
			ItemEntry.SetStackSize(OldStackSize - EffectiveNumToRemove);
			NotifyItemChanged(ItemEntry, OldStackSize, ItemEntry.GetStackSize());
		}

		NumRemoved += EffectiveNumToRemove;
		break;
	}

	// Actually remove now
	if (ItemToRemove.IsValid())
	{
		// Also remove it from the slot
		if (FInventoryItemSlot* Slot = FindItemSlotByItemId(ItemToRemove, FGameplayTag()))
		{
			Slot->UnoccupySlot();
		}

		InventoryList.RemoveItemFromList(ItemToRemove);
	}

	return NumRemoved;
}

void AInventoryBase::RemoveAllItems()
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_RemoveAllItems)
	for (auto It = InventoryList.CreateIterator(); It; ++It)
	{
		RemoveItemById(It->GetItemId());
	}
}

void AInventoryBase::Server_RemoveItem_Implementation(
	const UItemInstanceBase* Item,
	int32 NumToRemove)
{
	RemoveItem(Item, NumToRemove);
}

void AInventoryBase::Server_GiveItem_Implementation(
	const UItemDefinitionBase* ItemDefinition,
	int32 NumToGive,
	UObject* SourceObject,
	FGameplayTag PreferredGroup)
{
	int32 OutNumCouldNotGive;
	GiveItem(ItemDefinition, NumToGive, SourceObject, PreferredGroup, OutNumCouldNotGive);
}

UItemInstanceBase* AInventoryBase::PlaceItemInSlot(
	const UItemDefinitionBase* ItemDefinition,
	int32 NumToGive,
	UObject* SourceObject,
	const FInventorySlotId& SlotId,
	const FGameplayTag& GroupTag,
	int32& OutNumCouldNotGive)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_PlaceItemInSlot)

	OutNumCouldNotGive = NumToGive;
	UItemInstanceBase* Result = nullptr;
	FInventoryItemEntry ThisItem = FInventoryItemEntry(ItemDefinition, NumToGive, SourceObject);

	if (!HasAuthority())
	{
		UE_LOG(LogItemization, Warning, NET_TEXT(this, "Attempted to PlaceItemInSlot on a non authoritative connect. Not allowed!"))
		return Result;
	}

	if (!IsValid(ItemDefinition))
	{
		ITEMIZATION_LOG(Warning, "Attempted to PlaceItemInSlot with an invalid ItemDefinition.")
		return Result;
	}

	int32 CreatedStackSize;
	FInventoryItemId NewItemId;
	UItemInstanceBase* NewItem = AttemptCreateNewStack(ThisItem, NewItemId, SlotId, GroupTag, OutNumCouldNotGive, CreatedStackSize);
	if (IsValid(NewItem))
	{
		// Subtract stack size from excess
		OutNumCouldNotGive -= CreatedStackSize;
		Result = NewItem;
	}

	return Result;
}

void AInventoryBase::Server_PlaceItemInSlot_Implementation(
	const UItemDefinitionBase* ItemDefinition,
	int32 NumToGive,
	UObject* SourceObject,
	const FInventorySlotId& SlotId,
	const FGameplayTag& GroupTag)
{
	int32 OutNumCouldNotGive;
	PlaceItemInSlot(ItemDefinition, NumToGive, SourceObject, SlotId, GroupTag, OutNumCouldNotGive);
}

void AInventoryBase::SwapItemSlots(
	const FInventorySlotId& SlotA, const FGameplayTag& GroupTagA,
	const FInventorySlotId& SlotB, const FGameplayTag& GroupTagB)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_SwapItemSlots)

	if (!HasAuthority())
	{
		UE_LOG(LogItemization, Warning, NET_TEXT(this, "Attempted to SwapItemSlots on a non authoritative connect. Not allowed!"))
		return;
	}

	if (!SlotA.IsValid() ||
		!GroupTagA.IsValid() ||
		!SlotB.IsValid() ||
		!GroupTagB.IsValid())
	{
		ITEMIZATION_LOG(Warning, "Attempted to SwapItemSlots with invalid params.")
		return;
	}

	SlotList.SwapSlotsContent(SlotA, GroupTagA, SlotB, GroupTagB);
}

void AInventoryBase::Server_SwapItemSlots_Implementation(
	FInventorySlotId SlotA, FGameplayTag GroupTagA,
	FInventorySlotId SlotB, FGameplayTag GroupTagB)
{
	SwapItemSlots(SlotA, GroupTagA, SlotB, GroupTagB);
}

bool AInventoryBase::Server_SwapItemSlots_Validate(
	FInventorySlotId SlotA, FGameplayTag GroupTagA,
	FInventorySlotId SlotB, FGameplayTag GroupTagB)
{
	return SlotA.IsValid() && SlotB.IsValid();
}

AActor* AInventoryBase::DropItem(const UItemInstanceBase* Item, int32 NumToDrop)
{
	return DropItem(Item->GetItemId(), NumToDrop);
}

AActor* AInventoryBase::DropItem(const FInventoryItemId& ItemId, int32 NumToDrop)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_DropItem)

	FInventoryItemEntry* ItemToDrop = FindItemEntryById(ItemId);
	if (ItemToDrop == nullptr)
	{
		ITEMIZATION_LOG(Warning, "Attempted to DropItem with an invalid ItemId %d.", ItemId.Get())
		return nullptr;
	}

	FInventoryItemEntry EntryCopy = *ItemToDrop;
	EntryCopy.SetItemInstance(ItemToDrop->GetItemInstance());

	//@TODO: Change outer to the new pickup actor

	// Remove item
	int32 NumRemoved = RemoveItemById(ItemId, NumToDrop);
	EntryCopy.SetStackSizeNoDirty(NumRemoved);

	FPickupCreationData PickupCreationData = MakePickupCreationData(EntryCopy);
	if (!PickupCreationData.IsValid())
	{
		ITEMIZATION_LOG(Warning, "Couldn't create pickup creation data for item %s", *EntryCopy.GetDebugString())
		return nullptr;
	}

	return SpawnPickupActor(PickupCreationData);
}

void AInventoryBase::Server_DropItem_Implementation(FInventoryItemId ItemId, int32 NumToDrop)
{
	DropItem(ItemId, NumToDrop);
}

bool AInventoryBase::Server_DropItem_Validate(FInventoryItemId ItemId, int32 NumToDrop)
{
	return ItemId.IsValid();
}

TArray<AActor*> AInventoryBase::DropAllItems()
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_DropAllItems)
	TArray<AActor*> Drops;
	for (auto It = InventoryList.CreateIterator(); It; ++It)
	{
		FInventoryItemEntry& Item = *It;
		Drops.Add(DropItem(Item.GetItemId()));
	}

	return MoveTemp(Drops);
}

FInventoryItemEntry AInventoryBase::CreateNewItemEntry(
	const UItemDefinitionBase* InItemDefinition,
	int32 InCount,
	UObject* InSourceObject) const
{
	FInventoryItemEntry NewEntry = FInventoryItemEntry(InItemDefinition, InCount, InSourceObject);

	// Init the durability if we have
	if (NewEntry.GetItemDefinition()->HasTrait(UItemizationCoreSettings::Get()->HasDurabilityTag))
	{
		NewEntry.SetDurability(NewEntry.GetItemDefinition()->GetMaxDurability());
	}

	return NewEntry;
}

FInventoryItemEntry* AInventoryBase::FindItemEntryById(const FInventoryItemId& ItemId) const
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_FindItemEntryById)
	return InventoryList.FindItemEntryById(ItemId);
}

FInventoryItemEntry* AInventoryBase::FindFirstItemEntryByDefinition(const UItemDefinitionBase* ItemDefinition) const
{
	return InventoryList.FindFirstItemEntryByDefinition(ItemDefinition);
}

UItemInstanceBase* AInventoryBase::GetItem(const FInventoryItemId& ItemId) const
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_FindItemInstanceById)
	return InventoryList.FindItemInstanceById(ItemId);
}

FInventoryItemSlot* AInventoryBase::FindItemSlotBySlotId(const FInventorySlotId& SlotId, const FGameplayTag& GroupTag) const
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_FindItemSlotById)
	return SlotList.FindItemSlotBySlotId(SlotId, GroupTag);
}

FInventoryItemSlot* AInventoryBase::FindItemSlotByItemId(const FInventoryItemId& ItemId, const FGameplayTag& GroupTag) const
{
	return SlotList.FindItemSlotByItemId(ItemId, GroupTag);
}

FGameplayTag AInventoryBase::GetInventoryGroupTag(const FInventoryItemEntry& ItemEntry) const
{
	return SlotList.GetInventoryGroupForItemId(ItemEntry.GetItemId());
}

FGameplayTag AInventoryBase::GetInventoryGroupTag(const FInventoryItemId& ItemId) const
{
	return SlotList.GetInventoryGroupForItemId(ItemId);
}

TArray<FGameplayTag> AInventoryBase::GetAllItemSlotGroupTags() const
{
	return SlotList.GetAllItemGroupTags();
}

TArray<const FInventoryItemSlot*> AInventoryBase::GetItemSlotsInGroup(const FGameplayTag& GroupTag) const
{
	return SlotList.GetItemSlotsInGroup(GroupTag);
}

FInventoryItemSlot* AInventoryBase::GetNextAvailableItemSlot(
	const FInventoryItemEntry& ItemEntry,
	const FGameplayTag& GroupTag) const
{
	for (const FInventoryItemSlot& Slot : SlotList)
	{
		// Check for matching group, but only if the group tag is valid.
		// Otherwise we'll search all groups
		if (!Slot.GetGroupTag().MatchesTagExact(GroupTag) && GroupTag.IsValid())
		{
			continue;
		}

		// If it's occupied, check if we can combine them
		// Maybe in this slot, there is ItemA, but we want to add another ItemA,
		// so we could have 2x ItemA in this slot!
		if (!CanPlaceItemInSlot(ItemEntry, Slot))
		{
			continue;
		}

		return const_cast<FInventoryItemSlot*>(&Slot);
	}

	return nullptr;
}

int32 AInventoryBase::GetNumRowsInGroup(FGameplayTag GroupTag) const
{
	if (IsValid(InventoryConfigAsset))
	{
		for (const auto& Config : InventoryConfigAsset->InventoryGroupConfigs)
		{
			if (Config.GroupType.MatchesTagExact(GroupTag))
			{
				return Config.NumItemRows;
			}
		}
	}

	return 0;
}

int32 AInventoryBase::GetNumColumnsInGroup(FGameplayTag GroupTag) const
{
	if (IsValid(InventoryConfigAsset))
	{
		for (const auto& Config : InventoryConfigAsset->InventoryGroupConfigs)
		{
			if (Config.GroupType.MatchesTagExact(GroupTag))
			{
				return Config.NumItemColumns;
			}
		}
	}

	return 0;
}

UItemInstanceBase* AInventoryBase::GetItemInSlot(const FInventorySlotId& SlotId, const FGameplayTag& GroupTag) const
{
	if (FInventoryItemSlot* Slot = SlotList.FindItemSlotBySlotId(SlotId, GroupTag))
	{
		return GetItem(Slot->GetItemId());
	}

	return nullptr;
}

bool AInventoryBase::ContainsItem(const UItemInstanceBase* ItemInstance) const
{
	return InventoryList.ContainsItem(ItemInstance);
}

bool AInventoryBase::ContainsItem(const FInventoryItemId& ItemId) const
{
	return InventoryList.ContainsItem(ItemId);
}

FPickupCreationData AInventoryBase::MakePickupCreationData(const FInventoryItemEntry& ItemEntry) const
{
	check(ItemEntry.GetItemDefinition());

	if (const FItemComponentData_PickupBase* PickupData =
		ItemEntry.GetItemDefinition()->GetItemData<FItemComponentData_PickupBase>())
	{
		return PickupData->GetPickupCreationData(ItemEntry, this);
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
			ITEMIZATION_LOG(Warning, "Item can only have a single stack, but the a stack of the same type already exists.")
			return false;
		}
	}

	return true;
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

	// Items with durability should usually have a max stack size of 1
	// But we need to accumulate that
	if (This.GetItemDefinition()->HasTrait(UItemizationCoreSettings::Get()->HasDurabilityTag))
	{
		// If one of the items is "used" we cannot combine them anymore
		if (This.GetDurability() < This.GetItemDefinition()->GetMaxDurability() ||
			Other.GetDurability() < This.GetItemDefinition()->GetMaxDurability())
		{
			return false;
		}
	}

	return true;
}

bool AInventoryBase::CanRemoveItem(const FInventoryItemEntry& ItemEntry) const
{
	// Check for the trait that allows an empty stack, if present, not allow removing
	return ItemEntry.GetItemDefinition()->HasTrait(UItemizationCoreSettings::Get()->AllowEmptyStackTag) == false;
}

bool AInventoryBase::CanPlaceItemInSlot(const FInventoryItemEntry& ItemEntry, const FInventoryItemSlot& ItemSlot) const
{
	if (!IsValid(ItemEntry.GetItemDefinition()))
	{
		return false;
	}

	//@TODO: Swap logic ?
	if (ItemSlot.IsOccupied())
	{
		if (const FInventoryItemEntry* ItemEntryInSlot = ItemSlot.GetItemEntryInSlot())
		{
			if (!CanCombineItems(ItemEntry, *ItemEntryInSlot))
			{
				ITEMIZATION_LOG(Warning, "Can't place item '%s' in slot '%s' as the slot is already occupied by '%s'",
					*ItemEntry.GetDebugString(),
					*ItemSlot.GetDebugString(),
					*ItemSlot.GetItemId().ToString())

				return false;
			}
		}
	}

	// Check for disallowed item slot data
	if (const FItemComponentData_DisallowInventorySlot* DisallowSlotData =
		ItemEntry.GetItemDefinition()->GetItemData<FItemComponentData_DisallowInventorySlot>())
	{
		if (ItemSlot.GetSlotTags().HasAnyExact(DisallowSlotData->DisallowedSlotTags) ||
			ItemSlot.GetGroupTag().MatchesAnyExact(DisallowSlotData->DisallowedSlotTags))
		{
			ITEMIZATION_LOG(Warning, "Can't place item '%s' in slot '%s' as it has disallowed slot tags (%s)",
				*ItemEntry.GetDebugString(),
				*ItemSlot.GetDebugString(),
				*DisallowSlotData->DisallowedSlotTags.ToStringSimple())
			return false;
		}
	}

	return true;
}

void AInventoryBase::OnGiveItem(FInventoryItemEntry& ItemEntry)
{
	if (!ensure(ItemEntry.IsValid()))
	{
		return;
	}

	// Notify the instance about being added to the inventory
	ItemEntry.GetItemInstance()->OnAddedToInventory(ItemEntry, this);

	// Notify the item data
	for (const FItemComponentData* ItemData : ItemEntry.GetItemDefinition()->GetDataList())
	{
		if (ItemData != nullptr)
		{
			ItemData->OnItemGiven(ItemEntry, this);
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
	ItemEntry.GetItemInstance()->OnRemovedInventory(ItemEntry, this);

	// Notify the item data
	for (const FItemComponentData* ItemData : ItemEntry.GetItemDefinition()->GetDataList())
	{
		if (ItemData != nullptr)
		{
			ItemData->OnItemRemoved(ItemEntry, this);
		}
	}

	// Broadcast the remove event
	NotifyItemRemoved(ItemEntry, ItemEntry.GetLastObservedStackSize(), 0);
}

void AInventoryBase::OnAddItemSlot(FInventoryItemSlot& InventorySlot)
{
	// Broadcast the add event
	NotifyItemSlotAdded(InventorySlot, FInventoryItemId::InvalidId, InventorySlot.GetItemId());
}

void AInventoryBase::OnRemoveItemSlot(FInventoryItemSlot& InventorySlot)
{
	// Broadcast the remove event
	NotifyItemSlotRemoved(InventorySlot, InventorySlot.GetItemId(), FInventoryItemId::InvalidId);
}

void AInventoryBase::OnItemSlotChanged(
	FInventoryItemSlot& InventorySlot,
	const FInventoryItemId& LastItemId,
	const FInventoryItemId& NewItemId)
{
	// Broadcast the change event
	NotifyItemSlotChanged(InventorySlot, LastItemId, NewItemId);
}

void AInventoryBase::NotifyItemChanged(
	const FInventoryItemEntry& ItemThatChanged,
	const int32& LastStackSize,
	const int32& NewStackSize) const
{
	OnItemChangedDelegate.Broadcast(ItemThatChanged, LastStackSize, NewStackSize);
}

void AInventoryBase::NotifyItemAdded(
	const FInventoryItemEntry& ItemThatWasAdded,
	const int32& LastStackSize,
	const int32& NewStackSize) const
{
	OnItemAddedDelegate.Broadcast(ItemThatWasAdded, LastStackSize, NewStackSize);
}

void AInventoryBase::NotifyItemRemoved(
	const FInventoryItemEntry& ItemThatWasRemoved,
	const int32& LastStackSize,
	const int32& NewStackSize) const
{
	OnItemRemovedDelegate.Broadcast(ItemThatWasRemoved, LastStackSize, NewStackSize);
}

void AInventoryBase::NotifyItemSlotChanged(
	const FInventoryItemSlot& SlotThatChanged,
	const FInventoryItemId& LastItemInSlot,
	const FInventoryItemId& NewItemInSlot) const
{
	OnItemSlotChangedDelegate.Broadcast(SlotThatChanged, LastItemInSlot, NewItemInSlot);
}

void AInventoryBase::NotifyItemSlotAdded(
	const FInventoryItemSlot& SlotThatWasAdded,
	const FInventoryItemId& LastItemInSlot,
	const FInventoryItemId& NewItemInSlot) const
{
	OnItemSlotAddedDelegate.Broadcast(SlotThatWasAdded, LastItemInSlot, NewItemInSlot);
}

void AInventoryBase::NotifyItemSlotRemoved(
	const FInventoryItemSlot& SlotThatWasRemoved,
	const FInventoryItemId& LastItemInSlot,
	const FInventoryItemId& NewItemInSlot) const
{
	OnItemSlotRemovedDelegate.Broadcast(SlotThatWasRemoved, LastItemInSlot, NewItemInSlot);
}

UItemInstanceBase* AInventoryBase::CreateNewItemInstance(FInventoryItemEntry& ItemEntry)
{
	checkf(ItemEntry.GetItemInstance() == nullptr, TEXT("ItemEntry %s already has an instance!"),
		*ItemEntry.GetDebugString())

	const UItemDefinitionBase* ItemDefinition = ItemEntry.GetItemDefinition();
	check(ItemDefinition)

	// Get the item instance class
	const UClass* Class = ItemDefinition->GetItemInstanceClass().LoadSynchronous();
	if (Class == nullptr)
	{
		Class = UItemInstanceBase::StaticClass();
	}

	// Create the instance
	UItemInstanceBase* NewInstance = NewObject<UItemInstanceBase>(this, Class);
	check(NewInstance)

	AddReplicatedItemInstance(NewInstance);
	ItemEntry.SetItemInstance(NewInstance);

	ITEMIZATION_LOG(Verbose, "%s created a new item instance %s for %s.",
		*GetNameSafe(GetOwner()), *GetNameSafe(NewInstance), *ItemEntry.GetDebugString())

	return NewInstance;
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

void AInventoryBase::MarkItemSlotDirty(FInventoryItemSlot& ItemSlot, bool bWasAddOrChange, bool bForceMarkDirty)
{
	if (HasAuthority())
	{
		if (bWasAddOrChange || bForceMarkDirty)
		{
			SlotList.MarkItemDirty(ItemSlot);
		}
		else
		{
			SlotList.MarkArrayDirty();
		}
	}
	else
	{
		// Client-side. mark the entire array dirty so it will be replicated
		SlotList.MarkArrayDirty();
	}
}

void AInventoryBase::IncrementInventoryLock()
{
	InventoryLockCount++;
}

void AInventoryBase::DecrementInventoryLock()
{
	if (--InventoryLockCount == 0)
	{
		constexpr bool bPendingClearAll = false;
		if (bPendingClearAll)
		{

		}
		else if (!ItemPendingAdds.IsEmpty() || !ItemPendingRemoves.IsEmpty())
		{
			FInventoryLockActiveChange ActiveChange(*this, ItemPendingAdds, ItemPendingRemoves);
			for (auto& Entry : ActiveChange.Adds)
			{
				//@TODO: GiveItem
			}

			for (auto& Id : ActiveChange.Removes)
			{
				//@TODO: RemoveItem
			}
		}
	}
}

void AInventoryBase::AddPendingItemAdd(const FInventoryItemEntry& ItemEntry)
{
	ItemPendingAdds.Add(ItemEntry);
}

void AInventoryBase::AddPendingItemRemove(const FInventoryItemId& ItemId)
{
	ItemPendingRemoves.Add(ItemId);
}

bool AInventoryBase::ReplicateSubobjects(
	UActorChannel* Channel,
	FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (const auto& Entry : InventoryList)
	{
		if (IsValid(Entry.GetItemInstance()))
		{
			WroteSomething |= Channel->ReplicateSubobject(Entry.GetItemInstance(), *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void AInventoryBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.Condition = COND_ReplayOrOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryList, SharedParams)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, SlotList, SharedParams)

	SharedParams.Condition = COND_InitialOnly;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryConfigAsset, SharedParams)
}

void AInventoryBase::PreNetReceive()
{
	Super::PreNetReceive();
}

void AInventoryBase::PostNetReceive()
{
	Super::PostNetReceive();
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

	Super::EndPlay(EndPlayReason);
}

void AInventoryBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Register us a receiver so we can add component requests to this shit.
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AInventoryBase::CombineItems(FInventoryItemEntry& This, FInventoryItemEntry& Other, int32& OutCouldNotCombine)
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

	ITEMIZATION_LOG(Log, "CombineItems: Combined '%s' [old: %d -> new: %d] amd '%s' [old: %d -> new: %d]",
		*This.GetDebugString(), ThisStackSize, ThisNewStackSize,
		*Other.GetDebugString(), OtherStackSize, OtherNewStackSize)

	OutCouldNotCombine = FMath::Max(0, ThisStackSize - StacksToBeAdded);
}

UItemInstanceBase* AInventoryBase::AttemptCreateNewStack(
	FInventoryItemEntry& ItemEntry,
	FInventoryItemId& OutItemId,
	const FGameplayTag& GroupTag,
	const int32& RemainingStacks,
	int32& OutCreatedStackSize)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_CreateNewStack);

	OutCreatedStackSize = 0;

	// Make sure we have at least one remaining stack
	if (RemainingStacks <= 0)
	{
		return nullptr;
	}

	bool bForceIntoOverflow = ItemEntry.GetItemDefinition()->HasTrait(UItemizationCoreSettings::Get()->ForceIntoOverflowTag);

	// If we aren't going into overflow, try to find an item slot
	FInventoryItemSlot* ItemSlot = nullptr;
	if (!bForceIntoOverflow)
	{
		ItemSlot = GetNextAvailableItemSlot(ItemEntry, GroupTag);

		// If no slot was found, we can't add the item
		if (ItemSlot == nullptr)
		{
			return nullptr;
		}
	}

	// Sometimes we may not be able to create a new stack.
	// E.g., if the item is restricted to a single stack and we already have one
	if (!CanCreateNewStack(ItemEntry))
	{
		return nullptr;
	}

	if (!bForceIntoOverflow)
	{
		if (!CanPlaceItemInSlot(ItemEntry, *ItemSlot))
		{
			return nullptr;
		}
	}

	// If the slot was already occupied, but passed CanPlaceItemInSLot() we now combine them
	if (!bForceIntoOverflow && ItemSlot->IsOccupied())
	{
		int32 NumCouldNotCombine;
		FInventoryItemEntry* ItemInSlot = ItemSlot->GetItemEntryInSlot();
		CombineItems(ItemEntry, *ItemInSlot, NumCouldNotCombine);
		OutCreatedStackSize = RemainingStacks - NumCouldNotCombine;

		return ItemInSlot->GetItemInstance();
	}
	else
	{
		// Subtract the new stack size from the excess
		const int32 NewStackSize = FMath::Min(RemainingStacks, ItemEntry.GetItemDefinition()->GetMaxStackSize());
		OutCreatedStackSize = NewStackSize;

		// Create a copy of the item entry and update its stack size
		FInventoryItemEntry EntryCopy = ItemEntry;
		EntryCopy.SetStackSize(NewStackSize);

		// Add the item to the inventory and generate a new uid
		FInventoryItemEntry& NewItemEntry = InventoryList.AddItemToList(MoveTemp(EntryCopy));
		OutItemId = NewItemEntry.GetItemId();

		// Place the item id inside the slot and mark it dirty
		// The item slot might be nullptr if we force this item into overflow
		if (!bForceIntoOverflow)
		{
			ItemSlot->OccupySlot(NewItemEntry);
		}

		ITEMIZATION_LOG(Log, "Created new stack for %s count %d.",
			*GetNameSafe(NewItemEntry.GetItemInstance()), OutCreatedStackSize)

		return NewItemEntry.GetItemInstance();
	}
}

UItemInstanceBase* AInventoryBase::AttemptCreateNewStack(
	FInventoryItemEntry& ItemEntry,
	FInventoryItemId& OutItemId,
	const FInventorySlotId& SlotId,
	const FGameplayTag& GroupTag,
	const int32& RemainingStacks,
	int32& OutCreatedStackSize)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_CreateNewStack);

	OutCreatedStackSize = 0;

	// Make sure we have at least one remaining stack
	if (RemainingStacks <= 0)
	{
		return nullptr;
	}

	bool bForceIntoOverflow = ItemEntry.GetItemDefinition()->HasTrait(UItemizationCoreSettings::Get()->ForceIntoOverflowTag);

	// If we aren't going into overflow, try to find an item slot
	FInventoryItemSlot* ItemSlot = nullptr;
	if (!bForceIntoOverflow)
	{
		ItemSlot = FindItemSlotBySlotId(SlotId, GroupTag);

		// If no slot was found, we can't add the item
		if (ItemSlot == nullptr)
		{
			return nullptr;
		}
	}

	// Sometimes we may not be able to create a new stack.
	// E.g., if the item is restricted to a single stack and we already have one
	if (!CanCreateNewStack(ItemEntry))
	{
		return nullptr;
	}

	// Check if the item can be placed in the specified slot
	if (!CanPlaceItemInSlot(ItemEntry, *ItemSlot))
	{
		return nullptr;
	}

	// If the slot was already occupied, but passed CanPlaceItemInSLot() we now combine them
	if (ItemSlot->IsOccupied())
	{
		int32 NumCouldNotCombine;
		FInventoryItemEntry* ItemInSlot = ItemSlot->GetItemEntryInSlot();
		CombineItems(ItemEntry, *ItemInSlot, NumCouldNotCombine);
		OutCreatedStackSize = RemainingStacks - NumCouldNotCombine;

		return ItemInSlot->GetItemInstance();
	}
	else
	{
		// Subtract the new stack size from the excess
		const int32 NewStackSize = FMath::Min(RemainingStacks, ItemEntry.GetItemDefinition()->GetMaxStackSize());
		OutCreatedStackSize = NewStackSize;

		// Create a copy of the item entry and update its stack size
		FInventoryItemEntry EntryCopy = ItemEntry;
		EntryCopy.SetStackSize(NewStackSize);

		// Add the item to the inventory and generate a new uid
		FInventoryItemEntry& NewItemEntry = InventoryList.AddItemToList(MoveTemp(EntryCopy));
		OutItemId = NewItemEntry.GetItemId();

		// Place the item id inside the slot and mark it dirty
		// The item slot might be nullptr if we force this item into overflow
		if (!bForceIntoOverflow)
		{
			ItemSlot->OccupySlot(NewItemEntry);
		}

		ITEMIZATION_LOG(Log, "Created new stack for %s count %d.",
			*GetNameSafe(NewItemEntry.GetItemInstance()), OutCreatedStackSize)

		return NewItemEntry.GetItemInstance();
	}
}

void AInventoryBase::AddReplicatedItemInstance(UItemInstanceBase* ItemInstance)
{
	if (IsUsingRegisteredSubObjectList())
	{
		AddReplicatedSubObject(ItemInstance, COND_None);
	}
}

void AInventoryBase::OnRep_InventoryList()
{
}
