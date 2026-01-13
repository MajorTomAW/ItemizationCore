// Author: Tom Werner (MajorT), 2025


#include "Items/InventoryItemSlot.h"

FString FInventoryItemSlot::GetDebugString() const
{
	return FString::Printf(TEXT("r: %u, c: %u"), GetRowIndex(), GetColumnIndex());
}

FInventorySlotContainer::FInventorySlotContainer()
	: OwningInventory(nullptr)
{
}

FInventorySlotContainer::FInventorySlotContainer(AInventoryBase* InOwningInventory)
	: OwningInventory(InOwningInventory)
{
}

FInventoryItemSlot* FInventorySlotContainer::FindItemSlotByHandle(const FInventorySlotHandle& SlotHandle) const
{
	for (auto& Slot : ItemSlots)
	{
		if (Slot != SlotHandle)
		{
			continue;
		}

		return const_cast<FInventoryItemSlot*>(&Slot);
	}

	return nullptr;
}

FInventoryItemSlot* FInventorySlotContainer::FindItemSlotByHandle(const FInventoryItemHandle& ItemHandle) const
{
	for (auto& Slot : ItemSlots)
	{
		if (Slot != ItemHandle)
		{
			continue;
		}

		return const_cast<FInventoryItemSlot*>(&Slot);
	}

	return nullptr;
}

TArray<FInventoryItemSlot*> FInventorySlotContainer::FindSlotsInGroup(const FGameplayTag& InGroupTag) const
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

TArray<FInventoryItemHandle> FInventorySlotContainer::FindItemHandlesInGroup(const FGameplayTag& InGroupTag) const
{
	TArray<FInventoryItemHandle> ItemHandles;

	for (auto& Slot : ItemSlots)
	{
		if (!Slot.GetGroupTag().MatchesTagExact(InGroupTag))
		{
			continue;
		}

		if (!Slot.GetItemHandle().IsValid())
		{
			continue;
		}

		ItemHandles.Add(Slot.GetItemHandle());
	}

	return ItemHandles;
}

TArray<FInventoryItemHandle> FInventorySlotContainer::GetAllItemHandles() const
{
	TArray<FInventoryItemHandle> ItemHandles;

	for (auto& Slot : ItemSlots)
	{
		if (!Slot.GetItemHandle().IsValid())
		{
			continue;
		}

		ItemHandles.Add(Slot.GetItemHandle());
	}

	return ItemHandles;
}

FInventoryItemSlot* FInventorySlotContainer::GetNextUnoccupiedItemSlotInGroup(const FGameplayTag& InGroupTag) const
{
	for (auto& Slot : ItemSlots)
	{
		if (!Slot.GetGroupTag().MatchesTagExact(InGroupTag))
		{
			continue;
		}

		// Valid means its occupied
		if (Slot.GetItemHandle().IsValid())
		{
			continue;
		}

		return const_cast<FInventoryItemSlot*>(&Slot);
	}

	return nullptr;
}

TArray<FGameplayTag> FInventorySlotContainer::GetAllItemGroups() const
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
