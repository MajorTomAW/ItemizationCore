// Author: Tom Werner (MajorT), 2025


#include "Items/InventoryItemSlot.h"

#include "ItemizationCoreLogChannels.h"
#include "Inventory/InventorySlotGroup.h"
#include "Inventory/SlottableInventory.h"
#include "Items/IInventoryItemInstanceInterface.h"
#include "Items/InventoryItemEntry.h"

FInventoryItemSlot::FInventoryItemSlot()
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

	// Fill in the data
	ItemId = ItemEntry.GetItemId();

	MarkSlotDirty();

	// Notify the inventory
	OwningInventory->OnItemSlotChanged(*this);
}

void FInventoryItemSlot::UnoccupySlot()
{
	ItemId.Reset();

	MarkSlotDirty();

	// Notify the inventory
	OwningInventory->OnItemSlotChanged(*this);
}

UObject* FInventoryItemSlot::GetItemInSlot() const
{
	if (OwningInventory.IsValid())
	{
		if (const auto* Entry = OwningInventory->FindItemEntryById(ItemId))
		{
			return Entry->GetItemInstance().GetObject();
		}
	}

	return nullptr;
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
		OwningInventory->MarkItemSlotDirty(*this, true);
	}
}

void FInventoryItemSlot::TryResolveItemInstance()
{
	/*if (ItemId.IsValid())
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
	}*/
}

void FInventoryItemSlot::SwapContents(FInventoryItemSlot& Other)
{
	if (this == &Other)
	{
		return;
	}

	// Mark both dirty as we now alter them
	MarkSlotDirty();
	Other.MarkSlotDirty();

	/*// Reset both item instances as we will resolve them after swapping again
	ItemInstance.Reset();
	Other.ItemInstance.Reset();*/

	// Swap the item ids
	Swap(ItemId, Other.ItemId);

	// Resolve the item instances again
	TryResolveItemInstance();
	Other.TryResolveItemInstance();

	//@TODO: Notify about the slot change?

	if (OwningInventory.IsValid())
	{
		OwningInventory->NotifyItemSlotChanged(*this);
		OwningInventory->NotifyItemSlotChanged(Other);
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
	// OwningInventory isn't replicated.
	// Therefore, we rely on the array serializer to provide a valid one
	if (!OwningInventory.IsValid())
	{
		OwningInventory = InArraySerializer.OwningInventory;
	}

	// Attempt to resolve the item instance
	TryResolveItemInstance();
}

void FInventoryItemSlot::PostReplicatedChange(const FInventorySlotList& InArraySerializer)
{
	// OwningInventory isn't replicated.
	// Therefore, we rely on the array serializer to provide a valid one
	if (!OwningInventory.IsValid())
	{
		OwningInventory = InArraySerializer.OwningInventory;
	}

	// Attempt to resolve the item instance
	TryResolveItemInstance();

	if (InArraySerializer.OwningInventory)
	{
		InArraySerializer.OwningInventory->OnItemSlotChanged(*this);
	}
}


FInventorySlotList::FInventorySlotList()
	: OwningInventory(nullptr)
{
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
	const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance) const
{
	for (auto& Slot : ItemSlots)
	{
		if (Slot.GetItemInSlot() != ItemInstance.GetObject())
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

FGameplayTag FInventorySlotList::GetInventoryGroupForItemId(const FInventoryItemId& ItemId) const
{
	if (const FInventoryItemSlot* Slot = FindItemSlotByItemId(ItemId))
	{
		return Slot->GetGroupTag();
	}

	return FGameplayTag();
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
