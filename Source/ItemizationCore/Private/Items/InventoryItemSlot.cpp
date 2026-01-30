// Author: Tom Werner (MajorT), 2025


#include "Items/InventoryItemSlot.h"

#include "Inventory/InventorySlotGroup.h"
#include "Inventory/SlottableInventory.h"
#include "Items/IInventoryItemInstanceInterface.h"
#include "Items/InventoryItemEntry.h"

FString FInventoryItemSlot::GetDebugString() const
{
	return FString::Printf(TEXT("r: %u, c: %u"), GetRowIndex(), GetColumnIndex());
}

void FInventoryItemSlot::AssignItemToSlot(const FInventoryItemEntry& ItemEntry)
{
	if (ItemId == ItemEntry.GetItemId())
	{
		return;
	}

	ItemId = ItemEntry.GetItemId();
	ItemInstance = ItemEntry.GetItemInstance().GetObject();

	MarkSlotDirty();
}

void FInventoryItemSlot::MarkSlotDirty()
{
	if (OwningInventory.IsValid())
	{
		OwningInventory->MarkItemSlotDirty(*this);
	}
}

void FInventoryItemSlot::TryResolveItemInstance()
{
	if (ItemId.IsValid())
	{
		bool bNeedsToRefreshInstance = false;
		if (ItemInstance.IsValid())
		{
			if (IInventoryItemInstanceInterface* ItemInstanceInterface = Cast<IInventoryItemInstanceInterface>(ItemInstance.Get()))
			{
				if (ItemInstanceInterface->GetItemEntry()->GetItemId() != ItemId)
				{
					// Instance is valid, but not of the same type
					bNeedsToRefreshInstance = true;
				}
			}
		}
		else
		{
			// No instance but valid id, we need to refresh
			bNeedsToRefreshInstance = true;
		}

		
		if (bNeedsToRefreshInstance)
		{
			ItemInstance = OwningInventory->FindItemInstanceById(ItemId).GetObject();
		}
	}
	else
	{
		ItemInstance = nullptr;
	}
}

void FInventoryItemSlot::SetOwningInventory(ASlottableInventory* InOwningInventory)
{
	OwningInventory = InOwningInventory;
}

void FInventoryItemSlot::PreReplicatedRemove(const FInventorySlotList& InArraySerializer)
{
}

void FInventoryItemSlot::PostReplicatedAdd(const FInventorySlotList& InArraySerializer)
{
	// Attempt to resolve the item instance
	TryResolveItemInstance();
}

void FInventoryItemSlot::PostReplicatedChange(const FInventorySlotList& InArraySerializer)
{
	// Attempt to resolve the item instance
	TryResolveItemInstance();
}


FInventorySlotList::FInventorySlotList(ASlottableInventory* InOwningInventory)
	: OwningInventory(InOwningInventory)
{
}

FInventoryItemSlot& FInventorySlotList::AddSlotToList(FInventoryItemSlot ItemSlot)
{
	FInventoryItemSlot& NewSlot = ItemSlots.Emplace_GetRef(ItemSlot);
	NewSlot.SetOwningInventory(OwningInventory);
	return NewSlot;
}

FInventoryItemSlot& FInventorySlotList::AddSlotToList_Defaulted()
{
	FInventoryItemSlot& NewSlot = ItemSlots.AddDefaulted_GetRef();
	NewSlot.SetOwningInventory(OwningInventory);
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
			It.RemoveCurrent();
			MarkArrayDirty();
			return true;
		}
	}

	return false;
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

FInventoryItemSlot* FInventorySlotList::GetNextUnoccupiedSlotInGroup(const FGameplayTag& InGroupTag) const
{
	for (const FInventoryItemSlot& Slot : ItemSlots)
	{
		// Check for matching group, but only if the group tag is valid.
		// Otherwise we'll search all groups
		if (!Slot.GetGroupTag().MatchesTagExact(InGroupTag) && InGroupTag.IsValid())
		{
			continue;
		}

		// Valid means its occupied
		if (Slot.IsOccupied())
		{
			continue;
		}

		return const_cast<FInventoryItemSlot*>(&Slot);
	}

	return nullptr;
}

FInventorySlotId FInventorySlotList::GetNextUnoccupiedSlotIdInGroup(const FGameplayTag& InGroupTag) const
{
	for (const FInventoryItemSlot& Slot : ItemSlots)
	{
		// Check for matching group, but only if the group tag is valid.
		// Otherwise we'll search all groups
		if (!Slot.GetGroupTag().MatchesTagExact(InGroupTag) && InGroupTag.IsValid())
		{
			continue;
		}

		// Valid means its occupied
		if (Slot.IsOccupied())
		{
			continue;
		}

		return Slot.GetSlotId();
	}

	return FInventorySlotId();
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
