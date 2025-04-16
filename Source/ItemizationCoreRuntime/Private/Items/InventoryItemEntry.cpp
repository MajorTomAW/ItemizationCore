// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/InventoryItemEntry.h"

#include "ItemizationLogChannels.h"
#include "Inventory/Inventory.h"
#include "Items/InventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryItemEntry)

////////////////////////////////////////////////////////////////////
/// FInventoryItemEntry

FInventoryItemEntry::FInventoryItemEntry()
	: ReplInstance(nullptr)
	, LocalInstance(nullptr)
	, Definition(nullptr)
	, SourceObject(nullptr)
	, StackCount(0)
	, LastObservedStackCount(INDEX_NONE)
	, PendingRemove(false)
{
}

FInventoryItemEntry::FInventoryItemEntry(
	UItemDefinition* InItemDefinition,
	int32 InStackCount,
	UObject* InSourceObject)
	: ReplInstance(nullptr)
	, LocalInstance(nullptr)
	, Definition(InItemDefinition)
	, SourceObject(InSourceObject)
	, StackCount(InStackCount)
	, LastObservedStackCount(INDEX_NONE)
	, PendingRemove(false)
{
}


FString FInventoryItemEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s [%s]"), *GetNameSafe(GetInstance()), *ItemHandle.ToString());
}

void FInventoryItemEntry::Reset()
{
	ReplInstance = LocalInstance= nullptr;
	Definition = nullptr;
	SourceObject = nullptr;
	StackCount = 0;
	LastObservedStackCount = INDEX_NONE;
	ItemHandle.Reset();
	PendingRemove = false;
}

void FInventoryItemEntry::PreReplicatedRemove(const FInventoryItemContainer& InArraySerializer)
{
	if (InArraySerializer.OwningInventory)
	{
		InArraySerializer.OwningInventory->OnRemoveItem(*this);
	}
}

void FInventoryItemEntry::PostReplicatedAdd(const FInventoryItemContainer& InArraySerializer)
{
	ITEMIZATION_DISPLAY("PostReplicatedAdd: %s by %s", *GetDebugString(), *GetNameSafe(InArraySerializer.OwningInventory));

	if (InArraySerializer.OwningInventory)
	{
		InArraySerializer.OwningInventory->OnGiveItem(*this);
	}
}

////////////////////////////////////////////////////////////////////
/// FInventoryItemContainer

FInventoryItemContainer::FInventoryItemContainer()
	: OwningInventory(nullptr)
{
}

FInventoryItemContainer::FInventoryItemContainer(AInventory* InOwningInventory)
	: OwningInventory(InOwningInventory)
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
