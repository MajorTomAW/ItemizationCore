// Author: Tom Werner (dc: majort), 2026


#include "InventorySlotList.h"

#include "InventoryBase.h"
#include "InventoryItemList.h"
#include "ItemizationCoreLogChannels.h"

FInventoryItemSlot::FInventoryItemSlot()
	: SlotId(FInventorySlotId::InvalidId)
{
}

FInventoryItemSlot::FInventoryItemSlot(uint32 RowIdx, uint32 ColIdx)
	: SlotId(RowIdx, ColIdx)
{
}

FString FInventoryItemSlot::GetDebugString() const
{
	return FString::Printf(TEXT("r: %u, c: %u"), GetRowIndex(), GetColumnIndex());
}

void FInventoryItemSlot::OccupySlot(const FInventoryItemEntry& ItemEntry)
{
	if (ItemId == ItemEntry.GetItemId())
	{
		return;
	}

	// Occupy
	FInventoryItemId OldItemId = ItemId;
	ItemId = ItemEntry.GetItemId();
	ItemInSlot = ItemEntry.GetItemInstance();
	MarkSlotDirty();

	// Notify the inventory
	OwningInventory->OnItemSlotChanged(*this, OldItemId, ItemId);
}

void FInventoryItemSlot::UnoccupySlot()
{
	if (!ItemId.IsValid())
	{
		return;
	}

	// Unoccupy
	FInventoryItemId OldItemId = ItemId;
	ItemId.Reset();
	ItemInSlot = nullptr;

	MarkSlotDirty();

	// Notify the inventory
	OwningInventory->OnItemSlotChanged(*this, OldItemId, ItemId);
}

UItemInstanceBase* FInventoryItemSlot::GetItemInSlot() const
{
	return ItemInSlot;
}

const FInventoryItemEntry* FInventoryItemSlot::GetItemEntryInSlot() const
{
	return (ensure(OwningInventory.IsValid()) && ItemId.IsValid()) ? OwningInventory->FindItemEntryById(ItemId) : nullptr;
}

FInventoryItemEntry* FInventoryItemSlot::GetItemEntryInSlot()
{
	return (ensure(OwningInventory.IsValid()) && ItemId.IsValid()) ? OwningInventory->FindItemEntryById(ItemId) : nullptr;
}

void FInventoryItemSlot::MarkSlotDirty()
{
	if (ensure(OwningInventory.IsValid()))
	{
		OwningInventory->MarkItemSlotDirty(*this, true);
	}
}

void FInventoryItemSlot::SwapContents(FInventoryItemSlot& Other)
{
	if (this == &Other)
	{
		return;
	}

	// mark both dirty as we now alter them
	MarkSlotDirty();
	Other.MarkSlotDirty();

	// Swap the item ids
	Swap(ItemId, Other.ItemId);
	Swap(ItemInSlot, Other.ItemInSlot);

	// Notify the inventory about the change
	if (ensure(OwningInventory.IsValid()))
	{
		OwningInventory->OnItemSlotChanged(*this, Other.ItemId, ItemId);
		OwningInventory->OnItemSlotChanged(Other, ItemId, Other.ItemId);
	}
}

void FInventoryItemSlot::SetOwningInventory(AInventoryBase* InOwningInventory)
{
	OwningInventory = InOwningInventory;
}

void FInventoryItemSlot::PreReplicatedRemove(const FInventorySlotList& InArraySerializer)
{
	if (ensure(InArraySerializer.OwningInventory))
	{
		UE_LOG(LogItemization, Verbose, NET_TEXT(InArraySerializer.OwningInventory, "%s InventoryItemSlot removed (Non-Auth): %s [item: %s]"),
			*GetNameSafe(InArraySerializer.OwningInventory->GetOwner()), *SlotId.ToString(), *ItemId.ToString())

		InArraySerializer.OwningInventory->OnRemoveItemSlot(*this);
	}
}

void FInventoryItemSlot::PostReplicatedAdd(const FInventorySlotList& InArraySerializer)
{
	if (ensure(InArraySerializer.OwningInventory))
	{
		UE_LOG(LogItemization, Verbose, NET_TEXT(InArraySerializer.OwningInventory, "%s InventoryItemSlot added (Non-Auth): %s [item %s]"),
			*GetNameSafe(InArraySerializer.OwningInventory->GetOwner()), *SlotId.ToString(), *ItemId.ToString())

		InArraySerializer.OwningInventory->OnAddItemSlot(*this);
	}
}

void FInventoryItemSlot::PostReplicatedChange(const FInventorySlotList& InArraySerializer)
{
	if (ensure(InArraySerializer.OwningInventory))
	{
		UE_LOG(LogItemization, Verbose, NET_TEXT(InArraySerializer.OwningInventory, "%s InventoryItemSlot changed (Non-Auth): %s [item: %s]"),
			*GetNameSafe(InArraySerializer.OwningInventory->GetOwner()), *SlotId.ToString(), *ItemId.ToString())

		// Try resolve the item
		if (ItemId.IsValid())
		{
			// Not found, but valid slot id, we have to wait for it to be resolved
			if (!::IsValid(ItemInSlot))
			{
				bWaitingOnItemInstance = true;
			}
			else
			{
				bWaitingOnItemInstance = false;
				InArraySerializer.OwningInventory->OnItemSlotChanged(*this, FInventoryItemId(), ItemId);
			}
		}
		else
		{
			InArraySerializer.OwningInventory->OnItemSlotChanged(*this, FInventoryItemId(), ItemId);
		}
	}
}

FInventorySlotList::FInventorySlotList()
	: OwningInventory(nullptr)
{
}

FInventorySlotList::FInventorySlotList(AInventoryBase* InOwningInventory)
	: OwningInventory(InOwningInventory)
{
}

FInventoryItemSlot& FInventorySlotList::AddSlotToList(FInventoryItemSlot ItemSlot)
{
	FInventoryItemSlot& NewSlot = ItemSlots.Emplace_GetRef(ItemSlot);
	NewSlot.SetOwningInventory(OwningInventory);

	MarkItemDirty(NewSlot);

	// Notify the inventory
	OwningInventory->OnAddItemSlot(NewSlot);

	return NewSlot;
}

FInventoryItemSlot& FInventorySlotList::AddSlotToList_Defaulted(const FGameplayTag& SlotGroup)
{
	FInventoryItemSlot& NewSlot = ItemSlots.AddDefaulted_GetRef();
	NewSlot.SetOwningInventory(OwningInventory);
	NewSlot.SetGroupTag(SlotGroup);

	MarkItemDirty(NewSlot);

	// Notify the inventory
	OwningInventory->OnAddItemSlot(NewSlot);

	return NewSlot;
}

bool FInventorySlotList::RemoveSlotFromList(FInventorySlotId SlotId)
{
	if (!SlotId.IsValid())
	{
		return false;
	}

	for (auto It = ItemSlots.CreateIterator(); It; ++It)
	{
		FInventoryItemSlot& ItemSlot = *It;
		if (ItemSlot.GetSlotId() == SlotId)
		{
			OwningInventory->OnRemoveItemSlot(ItemSlot);

			It.RemoveCurrent();
			MarkArrayDirty();
			return true;
		}
	}

	return false;
}

void FInventorySlotList::SwapSlotsContent(
	const FInventorySlotId& SlotIdA, const FGameplayTag& SlotGroupA,
	const FInventorySlotId& SlotIdB, const FGameplayTag& SlotGroupB) const
{
	FInventoryItemSlot* SlotA = FindItemSlotBySlotId(SlotIdA, SlotGroupA);
	FInventoryItemSlot* SlotB = FindItemSlotBySlotId(SlotIdB, SlotGroupB);

	// Make sure both slots are found
	if (!(ensure(SlotA) && ensure(SlotB)))
	{
		return;
	}

	// If both slots are unoccupied, we can skip
	if (SlotA->IsUnoccupied() && SlotB->IsUnoccupied())
	{
		return;
	}

	SlotA->SwapContents(*SlotB);
}

FInventoryItemSlot* FInventorySlotList::FindItemSlot(
	const UItemInstanceBase* ItemInstance) const
{
	// Worst case N squared
	for (auto& Slot : ItemSlots)
	{
		if (Slot.GetItemInSlot() != ItemInstance)
		{
			continue;
		}

		return const_cast<FInventoryItemSlot*>(&Slot);
	}

	return nullptr;
}

FInventoryItemSlot* FInventorySlotList::FindItemSlotBySlotId(
	const FInventorySlotId& SlotId,
	const FGameplayTag& GroupTag) const
{
	for (auto& Slot : ItemSlots)
	{
		if (Slot != SlotId)
		{
			continue;
		}

		if (GroupTag.IsValid() && !Slot.GetGroupTag().MatchesTagExact(GroupTag))
		{
			continue;
		}

		return const_cast<FInventoryItemSlot*>(&Slot);
	}

	return nullptr;
}

FInventoryItemSlot* FInventorySlotList::FindItemSlotByItemId(
	const FInventoryItemId& ItemId,
	const FGameplayTag& GroupTag) const
{
	for (auto& Slot : ItemSlots)
	{
		if (Slot.GetItemId() != ItemId)
		{
			continue;
		}

		if (GroupTag.IsValid() && !Slot.GetGroupTag().MatchesTag(GroupTag))
		{
			continue;
		}

		return const_cast<FInventoryItemSlot*>(&Slot);
	}

	return nullptr;
}

TArray<FInventoryItemSlot*> FInventorySlotList::GetItemSlotsInGroup(const FGameplayTag& GroupTag)
{
	TArray<FInventoryItemSlot*> Result;
	for (auto& Slot : ItemSlots)
	{
		if (Slot.GetGroupTag() == GroupTag)
		{
			Result.Add(&Slot);
		}
	}
	return MoveTemp(Result);
}

TArray<const FInventoryItemSlot*> FInventorySlotList::GetItemSlotsInGroup(const FGameplayTag& GroupTag) const
{
	TArray<const FInventoryItemSlot*> Result;
	for (auto& Slot : ItemSlots)
	{
		if (Slot.GetGroupTag() == GroupTag)
		{
			Result.Add(&Slot);
		}
	}
	return MoveTemp(Result);
}

TArray<FInventoryItemId> FInventorySlotList::GetItemIdsInGroup(const FGameplayTag& InGroupTag) const
{
	TArray<FInventoryItemId> ItemIds;

	for (const FInventoryItemSlot* Slot : GetItemSlotsInGroup(InGroupTag))
	{
		ItemIds.Add(Slot->GetItemId());
	}

	return MoveTemp(ItemIds);
}

TArray<FInventoryItemId> FInventorySlotList::GetAllItemIds() const
{
	TArray<FInventoryItemId> ItemIds;

	for (auto& Slot : ItemSlots)
	{
		if (!Slot.GetItemId().IsValid())
		{
			continue;
		}

		ItemIds.Add(Slot.GetItemId());
	}

	return MoveTemp(ItemIds);
}

TArray<FGameplayTag> FInventorySlotList::GetAllItemGroupTags() const
{
	TArray<FGameplayTag> GroupTags;
	for (auto& Slot : ItemSlots)
	{
		if (!Slot.GetGroupTag().IsValid())
		{
			continue;
		}

		if (GroupTags.Contains(Slot.GetGroupTag()))
		{
			continue;
		}

		GroupTags.Add(Slot.GetGroupTag());
	}

	return MoveTemp(GroupTags);
}

FGameplayTag FInventorySlotList::GetInventoryGroupForItemId(const FInventoryItemId& ItemId) const
{
	if (const FInventoryItemSlot* Slot = FindItemSlotByItemId(ItemId))
	{
		return Slot->GetGroupTag();
	}

	return FGameplayTag();
}

void FInventorySlotList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
}

void FInventorySlotList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FInventoryItemSlot& Slot = ItemSlots[Index];
		Slot.SetOwningInventory(OwningInventory);
	}
}

void FInventorySlotList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}
