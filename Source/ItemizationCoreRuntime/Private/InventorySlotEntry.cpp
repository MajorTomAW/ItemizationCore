// Copyright © 2024 Playton. All Rights Reserved.


#include "InventorySlotEntry.h"

#include "ActorComponents/InventorySlotManager.h"

//////////////////////////////////////////////////////////////////////////
/// FInventorySlotEntry

FInventorySlotEntry::FInventorySlotEntry()
	: SlotID(INDEX_NONE)
{
}

FInventorySlotEntry::FInventorySlotEntry(const FInventoryItemEntryHandle& InHandle, int32 InSlotIndex)
	: Handle(InHandle)
	, SlotID(InSlotIndex)
{
}

void FInventorySlotEntry::PreReplicatedRemove(const FInventorySlotContainer& InArraySerializer)
{
}

void FInventorySlotEntry::PostReplicatedAdd(const FInventorySlotContainer& InArraySerializer)
{
}

void FInventorySlotEntry::PostReplicatedChange(const FInventorySlotContainer& InArraySerializer)
{
}

//////////////////////////////////////////////////////////////////////////
/// FInventorySlotContainer

FInventorySlotContainer::FInventorySlotContainer()
	: Owner(nullptr)
{
}

FInventorySlotContainer::FInventorySlotContainer(UInventorySlotManager* InOwner)
	: Owner(InOwner)
{
}

void FInventorySlotContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Index : RemovedIndices)
	{
		FInventorySlotEntry& SlotEntry = Slots[Index];
		Owner->OnSlotEntryRemoved.Broadcast(Owner->GetInventoryManager(), Owner, SlotEntry, Index);
	}
}

void FInventorySlotContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FInventorySlotEntry& SlotEntry = Slots[Index];
		Owner->OnSlotEntryAdded.Broadcast(Owner->GetInventoryManager(), Owner, SlotEntry, Index);
	}
}

void FInventorySlotContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Index : ChangedIndices)
	{
		FInventorySlotEntry& SlotEntry = Slots[Index];
		Owner->OnSlotEntryChanged.Broadcast(Owner->GetInventoryManager(), Owner, SlotEntry, Index);
	}
}

