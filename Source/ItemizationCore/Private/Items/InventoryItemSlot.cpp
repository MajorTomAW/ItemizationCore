// Author: Tom Werner (MajorT), 2025


#include "Items/InventoryItemSlot.h"

#include "Inventory/InventorySlotGroup.h"

FString FInventoryItemSlot::GetDebugString() const
{
	return FString::Printf(TEXT("r: %u, c: %u"), GetRowIndex(), GetColumnIndex());
}

FInventorySlotList::FInventorySlotList()
	: OwningInventory(nullptr)
{
}

FInventorySlotList::FInventorySlotList(AInventoryBase* InOwningInventory)
	: OwningInventory(InOwningInventory)
{
}

FInventoryItemSlot* FInventorySlotList::FindItemSlotBySlotId(const FInventorySlotId& SlotId) const
{
	for (auto& Slot : ItemSlots)
	{
		if (Slot != SlotId)
		{
			continue;
		}

		return const_cast<FInventoryItemSlot*>(&Slot);
	}

	return nullptr;
}

FInventoryItemSlot* FInventorySlotList::FindItemSlotByItemId(const FInventoryItemId& ItemId) const
{
	for (auto& Slot : ItemSlots)
	{
		if (Slot != ItemId)
		{
			continue;
		}

		return const_cast<FInventoryItemSlot*>(&Slot);
	}

	return nullptr;
}

FInventoryItemSlotGroup* FInventorySlotList::FindItemSlotGroup(const FGameplayTag& GroupTag)
{
	return ItemSlotGroups.Find(GroupTag);
}

const FInventoryItemSlotGroup* FInventorySlotList::FindItemSlotGroup(const FGameplayTag& GroupTag) const
{
	return ItemSlotGroups.Find(GroupTag);
}

TArray<FInventoryItemSlot*> FInventorySlotList::FindSlotsInGroup(const FGameplayTag& InGroupTag) const
{
	if (const FInventoryItemSlotGroup* SlotGroup = ItemSlotGroups.Find(InGroupTag))
	{
		return SlotGroup->SlotList;
	}

	return {};
}

TArray<FInventoryItemId> FInventorySlotList::GetItemIdsInGroup(const FGameplayTag& InGroupTag) const
{
	TArray<FInventoryItemId> ItemIds;

	for (FInventoryItemSlot* Slot : FindSlotsInGroup(InGroupTag))
	{
		ItemIds.Add(Slot->GetItemId());
	}

	return ItemIds;
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

	return ItemIds;
}

FInventoryItemSlot* FInventorySlotList::GetNextUnoccupiedItemSlotInGroup(const FGameplayTag& InGroupTag) const
{
	for (auto& Slot : ItemSlots)
	{
		if (!Slot.GetGroupTag().MatchesTagExact(InGroupTag))
		{
			continue;
		}

		// Valid means its occupied
		if (Slot.GetItemId().IsValid())
		{
			continue;
		}

		return const_cast<FInventoryItemSlot*>(&Slot);
	}

	return nullptr;
}

TArray<FGameplayTag> FInventorySlotList::GetAllItemGroups() const
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

	return GroupTags;
}

void FInventorySlotList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Index : RemovedIndices)
	{
		FInventoryItemSlot& Slot = ItemSlots[Index];

		// Remove a single slot from the lookup map
		if (FInventoryItemSlotGroup* SlotGroup = ItemSlotGroups.Find(Slot.GetGroupTag()))
		{
			SlotGroup->SlotList.RemoveSingle(&Slot);
		}
	}
}

void FInventorySlotList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FInventoryItemSlot& Slot = ItemSlots[Index];
		FInventoryItemSlotGroup& SlotGroup = ItemSlotGroups.FindOrAdd(Slot.GetGroupTag());
		if (!SlotGroup.GroupTag.IsValid())
		{
			SlotGroup.GroupTag = Slot.GetGroupTag();
		}
		
		SlotGroup.SlotList.Add(&Slot);
	}
}

void FInventorySlotList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Index : ChangedIndices)
	{
		FInventoryItemSlot& Slot = ItemSlots[Index];
		if (FInventoryItemSlotGroup* SlotGroup = ItemSlotGroups.Find(Slot.GetGroupTag()))
		{
			// This only works if the indices are synced
			// And idk if that's the case yet
			SlotGroup->SlotList[Index] = &Slot;
		}
	}
}
