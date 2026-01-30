// Author: Tom Werner (MajorT), 2025


#include "Items/InventoryItemSlot.h"

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

FInventoryItemSlot* FInventorySlotList::FindItemSlotById(const FInventorySlotId& SlotId) const
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

FInventoryItemSlot* FInventorySlotList::FindItemSlotById(const FInventoryItemId& ItemId) const
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

TArray<FInventoryItemSlot*> FInventorySlotList::FindSlotsInGroup(const FGameplayTag& InGroupTag) const
{
	TArray<FInventoryItemSlot*> Slots;

	for (auto& Slot : ItemSlots)
	{
		if (!Slot.GetGroupTag().MatchesTagExact(InGroupTag))
		{
			continue;
		}

		Slots.Add(const_cast<FInventoryItemSlot*>(&Slot));
	}

	return Slots;
}

TArray<FInventoryItemId> FInventorySlotList::FindItemIdsInGroup(const FGameplayTag& InGroupTag) const
{
	TArray<FInventoryItemId> ItemIds;

	for (auto& Slot : ItemSlots)
	{
		if (!Slot.GetGroupTag().MatchesTagExact(InGroupTag))
		{
			continue;
		}

		if (!Slot.GetItemId().IsValid())
		{
			continue;
		}

		ItemIds.Add(Slot.GetItemId());
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
