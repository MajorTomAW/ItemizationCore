// Author: Tom Werner (dc: majort), 2026 January


#include "Inventory/SlottableInventory.h"

#include "ItemizationCoreLogChannels.h"
#include "ItemizationCoreStats.h"
#include "Items/Data/ItemComponentData_DisallowInventorySlot.h"
#include "Net/UnrealNetwork.h"

ASlottableInventory::ASlottableInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventorySlotList(this)
{
}

void ASlottableInventory::InitializeInventorySlots(const UInventoryConfigAsset* InConfig)
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

	ITEMIZATION_DISPLAY("Initializing inventory slots using '%s'", *InConfig->GetName())

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
				FInventoryItemSlot& NewSlot = InventorySlotList.AddSlotToList_Defaulted(GroupConfig.GroupType);
				NewSlot.SetGroupTag(GroupConfig.GroupType);
				NewSlot.SetSlotId(SlotId);

				ITEMIZATION_LOG("Added item slot [%s]", *SlotId.ToString())

				// See if we have slot tags
				if (const FGameplayTagContainer* SlotTags = GroupConfig.SlotTagMap.Find(SlotIndex))
				{
					NewSlot.SetSlotTags(*SlotTags);

					ITEMIZATION_LOG("Found slot tags for slot [%s: %s]: %s", *GroupConfig.GroupType.ToString(), *SlotId.ToString(), *SlotTags->ToString())
				}

				// Mark the slot dirty for replication
				InventorySlotList.MarkItemDirty(NewSlot);
				SlotIndex++;
			}
		}
	}
}

TInventoryOpPtr<FInventoryOp_PlaceItemInSlot> ASlottableInventory::PlaceItemInSlot(
	FInventoryOp_PlaceItemInSlot::FParams&& Params)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_PlaceItemInSlot);

	if (!Params.AreParamsValid())
	{
		ITEMIZATION_WARN("Called with invalid params [%s].", *Params.GetDebugString())
		return nullptr;
	}

	if (!HasAuthority())
	{
		ITEMIZATION_WARN("Called on an actor with no authority. Not allowed!")
		return nullptr;
	}

	/*
	if (Params.ResolveItemSlot(this) == nullptr)
	{
		ITEMIZATION_WARN("Unable to resolve item slot")
		return nullptr;
	}*/

	// Create the item entry and notify the item data about it
	if (!Params.ItemEntry)
	{
		if (!Params.ItemDefinition.IsValid())
		{
			return nullptr;
		}

		// Create a new item entry and store it in the params
		FInventoryItemEntry NewItemEntry = CreateItemEntry(Params.ItemDefinition.Get(), Params.NumItems, Params.SourceObject.Get());
		Params.ItemEntry = &NewItemEntry;
	}

	ITEMIZATION_LOG("Placing item (%s) in slot (%s) with count (%d) and source (%s) in inventory (%s)",
		*GetNameSafe(Params.ItemEntry->GetItemDefinition()),
		*Params.TargetSlotId.ToString(),
		Params.NumItems,
		*GetNameSafe(Params.SourceObject.Get()),
		*GetName())

	// Create the operation & process the operation
	TInventoryOpRef<FInventoryOp_PlaceItemInSlot> NewOp =
		MakeSharedOp<FInventoryOp_PlaceItemInSlot>(Params);
	ProcessPlaceItemInSlotOperation(NewOp);

	return NewOp;
}

TInventoryOpPtr<FInventoryOp_SwapItemSlots> ASlottableInventory::SwapItemSlots(
	FInventoryOp_SwapItemSlots::FParams&& Params)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_SwapItemSlots)

	if (!Params.AreParamsValid())
	{
		ITEMIZATION_WARN("Called with invalid params [%s].", *Params.GetDebugString())
		return nullptr;
	}

	if (!HasAuthority())
	{
		ITEMIZATION_WARN("Called on an actor with no authority. Not allowed!")
		return nullptr;
	}

	ITEMIZATION_LOG("Swapping item slots content from '%s' [%s] with '%s' [%s]",
		*Params.SourceSlotId.ToString(),
		*GetNameSafe(Params.SourceInventory.Get()),
		*Params.TargetSlotId.ToString(),
		*GetNameSafe(Params.TargetInventory.Get()))

	// Create the operation and process it
	TInventoryOpRef<FInventoryOp_SwapItemSlots> NewOp =
		MakeSharedOp<FInventoryOp_SwapItemSlots>(Params);
	ProcessSwapItemSlotsOperation(NewOp);

	return NewOp;
}

FInventorySlotId ASlottableInventory::GetNextUnoccupiedSlotIdInGroup(FGameplayTag GroupTag) const
{
	return InventorySlotList.GetNextUnoccupiedSlotIdInGroup(GroupTag);
}

FInventoryItemSlot* ASlottableInventory::GetNextUnoccupiedSlotInGroup(const FGameplayTag& GroupTag) const
{
	return InventorySlotList.GetNextUnoccupiedSlotInGroup(GroupTag);
}

int32 ASlottableInventory::GetNumRowsInGroup(FGameplayTag GroupTag) const
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

int32 ASlottableInventory::GetNumColumnsInGroup(FGameplayTag GroupTag) const
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

FInventoryItemSlot* ASlottableInventory::GetNextAvailableItemSlot(
	const FInventoryItemEntry& ItemEntry,
	const FGameplayTag& GroupTag) const
{
	for (const FInventoryItemSlot& Slot : InventorySlotList)
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

FInventoryItemSlot* ASlottableInventory::FindItemSlotBySlotId(const FInventorySlotId& SlotId, const FGameplayTag& GroupTag) const
{
	return InventorySlotList.FindItemSlotBySlotId(SlotId, GroupTag);
}

FInventoryItemSlot* ASlottableInventory::FindItemSlotByItemId(const FInventoryItemId& ItemId, const FGameplayTag& GroupTag) const
{
	return InventorySlotList.FindItemSlotByItemId(ItemId, GroupTag);
}

void ASlottableInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.Condition = COND_ReplayOrOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventorySlotList, SharedParams)

	SharedParams.Condition = COND_InitialOnly;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryConfigAsset, SharedParams)
}

void ASlottableInventory::OnRemoveItem(FInventoryItemEntry& ItemEntry)
{
	if (HasAuthority())
	{
		// If the item being removed is inside a slot, unoccpy the slot now
		if (FInventoryItemSlot* Slot = FindItemSlotByItemId(ItemEntry.GetItemId(), FGameplayTag()))
		{
			Slot->UnoccupySlot();
		}
	}

	Super::OnRemoveItem(ItemEntry);
}

bool ASlottableInventory::MatchesRemoveFilter(
	const FInventoryItemEntry& ItemEntry,
	FInventoryOp_RemoveItem::FParams& Params) const
{
	if (!Super::MatchesRemoveFilter(ItemEntry, Params))
	{
		return false;
	}

	// If we can't find the item slot the item is placed in, in the specified group, we cant remove !!
	if (!FindItemSlotByItemId(ItemEntry.GetItemId(), Params.GroupTag))
	{
		return false;
	}

	return true;
}

bool ASlottableInventory::AttemptCreateNewStack(
	FInventoryItemEntry& ItemEntry,
	FInventoryItemId& OutItemId,
	const int32& RemainingStacks,
	int32& OutCreatedStackSize,
	FInventoryOp_AdditiveBase::FAdditiveParamsBase* Params)
{
	int32 StackSize = RemainingStacks;
	FInventoryItemSlot* ItemSlot = nullptr;
	if (Params->GetParamsType() == FInventoryOp_PlaceItemInSlot::Name)
	{
		if (FInventoryOp_PlaceItemInSlot::FParams* SlotParams = static_cast<FInventoryOp_PlaceItemInSlot::FParams*>(Params))
		{
			// Use slot id from params
			ItemSlot = FindItemSlotBySlotId(SlotParams->TargetSlotId, SlotParams->GroupTag);

			ITEMIZATION_DISPLAY("using slot from op '%s'", ItemSlot ? *ItemSlot->GetDebugString() : TEXT("Invalid"))
		}
	}
	else if (Params->GetParamsType() == FInventoryOp_GiveItem::Name)
	{
		if (FInventoryOp_GiveItem::FParams* GiveParams = static_cast<FInventoryOp_GiveItem::FParams*>(Params))
		{
			// Try to find the next one
			ItemSlot = GetNextAvailableItemSlot(ItemEntry, GiveParams->GroupTag);

			ITEMIZATION_DISPLAY("no slot specified, will use next free one '%s'", ItemSlot ? *ItemSlot->GetDebugString() : TEXT("Invalid"))
		}
	}
	else
	{
		ITEMIZATION_WARN("Params of undefined type %s", *Params->GetParamsType().ToString())
	}

	// Make sure we have a valid slot.
	if (ItemSlot == nullptr)
	{
		ITEMIZATION_WARN("Couldn't find a valid item slot")
		return false;
	}

	// Sometimes we may not be able to create a new stack.
	// E.g., if the item is restricted to a single stack and we already have one
	if (!CanCreateNewStack(ItemEntry))
	{
		return false;
	}

	// Check if the item can be placed in the specified slot
	if (!CanPlaceItemInSlot(ItemEntry, *ItemSlot))
	{
		return false;
	}

	// If the slot is already occupied, but passed CanPlaceItemInSlot() we now combine them
	if (ItemSlot->IsOccupied())
	{
		int32 NumCouldNotCombine;
		CombineItems(ItemEntry, *ItemSlot->GetItemEntryInSlot(), NumCouldNotCombine);
		OutCreatedStackSize = StackSize - NumCouldNotCombine;
	}
	else
	{
		// Subtract the new stack size from the excess
		const int32 NewStackSize = FMath::Min(StackSize, ItemEntry.GetItemDefinition()->GetMaxStackSize());
		OutCreatedStackSize = NewStackSize;

		// Create a copy of the item entry and update its stack size
		FInventoryItemEntry EntryCopy = ItemEntry;
		EntryCopy.SetStackSize(NewStackSize);

		// Add the item to the inventory and generate a new uid
		FInventoryItemEntry& NewItemEntry = InventoryList.AddItemToList(MoveTemp(EntryCopy));
		OutItemId = NewItemEntry.GetItemId();

		// Also assign the item to the given slot
		ItemSlot->OccupySlot(NewItemEntry);

		ITEMIZATION_DISPLAY("Created new stack for '%s' count %d.",
			*GetNameSafe(ItemEntry.GetItemDefinition()), OutCreatedStackSize)
	}

	return true;
}

void ASlottableInventory::OnItemSlotChanged(const FInventoryItemSlot& ItemSlot)
{
	NotifyItemSlotChanged(ItemSlot);
}

void ASlottableInventory::NotifyItemSlotChanged(const FInventoryItemSlot& ItemSlot)
{
	OnItemSlotChangedDelegate.Broadcast(ItemSlot);
}

void ASlottableInventory::MarkItemSlotDirty(
	FInventoryItemSlot& ItemSlot,
	bool bWasAddOrChange,
	bool bForceMarkSlotDirty)
{
	if (HasAuthority())
	{
		if (bWasAddOrChange || bForceMarkSlotDirty)
		{
			InventorySlotList.MarkItemDirty(ItemSlot);
		}
		else
		{
			InventorySlotList.MarkArrayDirty();
		}
	}
	else
	{
		// Client-side, mark the entire array dirty so it will be replicated
		InventorySlotList.MarkArrayDirty();
	}
}

void ASlottableInventory::ProcessPlaceItemInSlotOperation(
	const TInventoryOpRef<FInventoryOp_PlaceItemInSlot>& PlaceItemInSlotOp)
{
	FInventoryOp_PlaceItemInSlot::FParams& Params = PlaceItemInSlotOp->Params;
	FInventoryOp_PlaceItemInSlot::FResult& Result = PlaceItemInSlotOp->Result;
	FInventoryItemEntry& ThisItem = *Params.ItemEntry;

	// Default to false, in case we early-out
	Result.bSuccess = false;
	Result.Excess = Params.NumItems;

	FInventoryItemId LastRelevantId = FInventoryItemId::InvalidId;

	// Attempt to place the item into the slot
	//@TODO: Design question ??
	//@TODO: Say we want to place 4x ItemA in slot (row: 0, col: 0) but ItemA has a max stack size of 5x and
	//@TODO: in that slot there already is 2x of ItemA
	//@TODO: Meaning we can only place 3x ItemA and would have 1x ItemA excess.
	//@TODO: So should we just go to the next slot to place the remaining 1x ItemA excess, or should we just say 1x ItemA
	//@TODO: couldn't be added and call it a day?
	int32 CreatedStackSize = 0;
	if (AttemptCreateNewStack(ThisItem, LastRelevantId, Result.Excess, CreatedStackSize, &Params))
	{
		// Subtract the stack size from the excess
		Result.Excess -= CreatedStackSize;
	}

	// Clamp excess to 0
	Result.Excess = FMath::Max(0, Result.Excess);
	Result.ItemId = LastRelevantId;
	Result.bSuccess = true;
}

bool ASlottableInventory::CanPlaceItemInSlot(
	const FInventoryItemEntry& ItemEntry,
	const FInventoryItemSlot& ItemSlot) const
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
				ITEMIZATION_WARN("Can't place item '%s' in slot '%s' as the slot is already occupied by '%s'",
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
			ITEMIZATION_WARN("Can't place item '%s' in slot '%s' as it has disallowed slot tags (%s)",
				*ItemEntry.GetDebugString(),
				*ItemSlot.GetDebugString(),
				*DisallowSlotData->DisallowedSlotTags.ToStringSimple())
			return false;
		}
	}

	return true;
}

void ASlottableInventory::ProcessSwapItemSlotsOperation(
	const TInventoryOpRef<FInventoryOp_SwapItemSlots>& SwapItemSlotsOp)
{
	FInventoryOp_SwapItemSlots::FParams& Params = SwapItemSlotsOp->Params;
	FInventoryOp_SwapItemSlots::FResult& Result = SwapItemSlotsOp->Result;

	// First, check if target and source inventory are this inventory
	// If they are, we can simplify things by a lot
	if (Params.IsSameInventory() && Params.SourceInventory == this)
	{
		InventorySlotList.SwapSlotsContent(Params.SourceSlotId, Params.SourceGroupTag, Params.TargetSlotId, Params.TargetGroupTag);
	}
}
