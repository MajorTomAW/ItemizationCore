// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/InventoryItemEntry.h"

#include "Items/InventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryItemEntry)

////////////////////////////////////////////////////////////////////
/// FInventoryItemEntry

FInventoryItemEntry::FInventoryItemEntry()
	: Handle(FInventoryItemHandle())
	, Instance(nullptr)
	, SourceObject(nullptr)
	, StackCount(0)
	, LastObservedStackCount(INDEX_NONE)
	, PendingRemove(false)
{
}

FInventoryItemEntry::FInventoryItemEntry(UObject* InSourceObject, int32 InStackCount)
	: Handle(FInventoryItemHandle())
	, Instance(nullptr)
	, SourceObject(InSourceObject)
	, StackCount(InStackCount)
	, LastObservedStackCount(INDEX_NONE)
	, PendingRemove(false)
{
}


FString FInventoryItemEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s (%d)"), *GetNameSafe(Instance), Handle);
}

void FInventoryItemEntry::PreReplicatedRemove(const FInventoryItemContainer& InArraySerializer)
{
}

void FInventoryItemEntry::PostReplicatedAdd(const FInventoryItemContainer& InArraySerializer)
{
}

////////////////////////////////////////////////////////////////////
/// FInventoryItemContainer

FInventoryItemContainer::FInventoryItemContainer()
{
}

void FInventoryItemContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
}

void FInventoryItemContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
}

void FInventoryItemContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}
