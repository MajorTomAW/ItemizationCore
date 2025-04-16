// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/Slots/InventorySlotEntry.h"

#include "ItemizationLogChannels.h"

/////////////////////////////////////////////////////////////////////
/// FInventorySlotEntry

FInventorySlotEntry::FInventorySlotEntry()
	: SlotHandle(FInventorySlotHandle())
	, bEnabled(true)
{
}

FInventorySlotEntry::FInventorySlotEntry(uint16 InSlotId)
	: bEnabled(true)
{
	SlotHandle.SetSlotId(InSlotId);
}

FInventorySlotEntry::FInventorySlotEntry(const FInventorySlotHandle& InItemHandle)
	: SlotHandle(InItemHandle)
	, bEnabled(true)
{
}


void FInventorySlotEntry::Reset()
{
	SlotHandle.Reset();
	bEnabled = true;
	SlotTag = FGameplayTag();
}

bool FInventorySlotEntry::IsValid() const
{
	return SlotHandle.IsSlotValid();
}

void FInventorySlotEntry::PreReplicatedRemove(const FInventorySlotContainer& InArraySerializer)
{
}

void FInventorySlotEntry::PostReplicatedAdd(const FInventorySlotContainer& InArraySerializer)
{
	ITEMIZATION_DISPLAY("Added slot %s", *GetHandle().ToString());
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
