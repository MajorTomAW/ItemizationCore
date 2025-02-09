// Copyright © 2024 Playton. All Rights Reserved.


#include "InventorySlotEntry.h"

//////////////////////////////////////////////////////////////////////////
/// FInventorySlotEntry

FInventorySlotEntry::FInventorySlotEntry()
	: Handle()
	, SlotIndex(INDEX_NONE)
{
}

FInventorySlotEntry::FInventorySlotEntry(const FInventoryItemEntryHandle& InHandle, int32 InSlotIndex)
	: Handle(InHandle)
	, SlotIndex(InSlotIndex)
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
{
}

void FInventorySlotContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
}

void FInventorySlotContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
}

void FInventorySlotContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}

