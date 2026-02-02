// Author: Tom Werner (MajorT), 2025


#include "Items/InventoryItemSlot.h"

#include "ItemizationCoreLogChannels.h"
#include "Inventory/InventorySlotGroup.h"
#include "Inventory/SlottableInventory.h"
#include "Items/IInventoryItemInstanceInterface.h"
#include "Items/InventoryItemEntry.h"

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

	ItemId = ItemEntry.GetItemId();
	ItemInstance = ItemEntry.GetItemInstance().GetObject();

	MarkSlotDirty();
}

void FInventoryItemSlot::UnoccupySlot()
{
	ItemId.Reset();
	ItemInstance.Reset();

	MarkSlotDirty();
}

UObject* FInventoryItemSlot::GetItemInSlot() const
{
	return ItemInstance.Get();
}

const FInventoryItemEntry* FInventoryItemSlot::GetItemEntryInSlot() const
{
	return OwningInventory.IsValid() ? OwningInventory->FindItemEntryById(GetItemId()) : nullptr;
}

FInventoryItemEntry* FInventoryItemSlot::GetItemEntryInSlot()
{
	return OwningInventory.IsValid() ? OwningInventory->FindItemEntryById(GetItemId()) : nullptr;
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


FInventorySlotList::FInventorySlotList()
	: OwningInventory(nullptr)
{
	ITEMIZATION_WARN("An inventory slot list was constructed without a default owning inventory actor. "
				  "This will result in undefined behavior.")
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

FInventoryItemSlot& FInventorySlotList::AddSlotToList_Defaulted(const FGameplayTag& SlotGroup)
{
	FInventoryItemSlot& NewSlot = ItemSlots.AddDefaulted_GetRef();
	NewSlot.SetOwningInventory(OwningInventory);
	NewSlot.SetGroupTag(SlotGroup);
	
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
		if (Slot != ItemId)
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

TArray<FInventoryItemSlot*> FInventorySlotList::GetItemSlotsInGroup(const FGameplayTag& GroupTag)
{
	TArray<FInventoryItemSlot*> Result;
	for (FInventoryItemSlot& Slot : ItemSlots)
	{
		if (Slot.GetGroupTag() == GroupTag)
		{
			Result.Add(&Slot);
		}
	}
	return Result;
}

TArray<const FInventoryItemSlot*> FInventorySlotList::GetItemSlotsInGroup(const FGameplayTag& GroupTag) const
{
	TArray<const FInventoryItemSlot*> Result;
	for (const FInventoryItemSlot& Slot : ItemSlots)
	{
		if (Slot.GetGroupTag() == GroupTag)
		{
			Result.Add(&Slot);
		}
	}
	return Result;
}

TArray<FInventoryItemId> FInventorySlotList::GetItemIdsInGroup(const FGameplayTag& InGroupTag) const
{
	TArray<FInventoryItemId> ItemIds;

	for (const FInventoryItemSlot* Slot : GetItemSlotsInGroup(InGroupTag))
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

	return GroupTags;
}

void FInventorySlotList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Index : RemovedIndices)
	{
		FInventoryItemSlot& Slot = ItemSlots[Index];
	}
}

void FInventorySlotList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FInventoryItemSlot& Slot = ItemSlots[Index];
	}
}

void FInventorySlotList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Index : ChangedIndices)
	{
		FInventoryItemSlot& Slot = ItemSlots[Index];
	}
}
