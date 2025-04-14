// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/Slots/InventorySlotEntry.h"

/////////////////////////////////////////////////////////////////////
/// FInventorySlotEntry

FInventorySlotEntry::FInventorySlotEntry()
	: Handle(FInventoryItemHandle())
	, bEnabled(true)
{
}

FInventorySlotEntry::FInventorySlotEntry(uint16 InSlotId)
	: bEnabled(true)
{
	Handle.SetSlotId(InSlotId);
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

///////////////////////////////////////////////////////////////////////
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
