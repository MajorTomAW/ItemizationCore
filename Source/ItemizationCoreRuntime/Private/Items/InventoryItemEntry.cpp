// Author: Tom Werner (MajorT), 2025


#include "Items/InventoryItemEntry.h"

#include "ItemizationGameplayTags.h"
#include "ItemizationLogChannels.h"
#include "Inventory/InventoryBase.h"
#include "Items/InventoryItemInstance.h"

FInventoryItemEntry::FInventoryItemEntry()
	: SlotNumber(INDEX_NONE)
	, LastObservedStackCount(INDEX_NONE)
	, bPendingRemove(false)
{
}

FInventoryItemEntry::FInventoryItemEntry(
	UItemDefinitionBase* InItemDefinition,
	int32 InStackSize,
	UObject* InSourceObj)
		: ItemDefinition(InItemDefinition)
		, SlotNumber(INDEX_NONE)
		, SourceObject(InSourceObj)
		, LastObservedStackCount(INDEX_NONE)
		, bPendingRemove(false)
{
	SetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize, InStackSize);
}

FString FInventoryItemEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s [%s]"), *GetNameSafe(GetItemInstance()), *ItemHandle.ToString());
}

void FInventoryItemEntry::DebugPrintStats() const
{
#if ENABLE_DRAW_DEBUG
	for (const auto& Pair : GetAllStats())
	{
		ITEMIZATION_LOG("\t%s: %s",
			*Pair.Key.ToString(),
			*FString::Printf(TEXT("%d"), Pair.Value));
	}
#endif
}

void FInventoryItemEntry::Reset()
{
	ReplicatedInstance = nullptr;
	NonReplicatedInstance = nullptr;
	ItemDefinition = nullptr;
	SourceObject = nullptr;
	//ItemData.Reset();
	SlotNumber = INDEX_NONE;
	LastObservedStackCount = INDEX_NONE;
	ItemHandle.Reset();
	bPendingRemove = false;
	TagCountMap.Reset();
}

UInventoryItemInstance* FInventoryItemEntry::GetItemInstance() const
{
	if (IsValid(ReplicatedInstance))
	{
		return ReplicatedInstance;
	}

	if (IsValid(NonReplicatedInstance))
	{
		return NonReplicatedInstance;
	}

	return nullptr;
}

void FInventoryItemEntry::SetReplicatedItemInstance(UInventoryItemInstance* InInstance)
{
	checkf(NonReplicatedInstance == nullptr,
		TEXT("You cannot set the replicated instance if a non-replicated instance [%s] already exists!"),
		*GetNameSafe(NonReplicatedInstance));
	
	ReplicatedInstance = InInstance;
}

void FInventoryItemEntry::SetNonReplicatedItemInstance(UInventoryItemInstance* InInstance)
{
	checkf(ReplicatedInstance == nullptr,
		TEXT("You cannot set the non-replicated instance if a replicated instance [%s] already exists!"),
		*GetNameSafe(ReplicatedInstance));

	NonReplicatedInstance = InInstance;
}

int32 FInventoryItemEntry::GetStatValue(const FGameplayTag& Tag) const
{
	if (TagCountMap.Contains(Tag))
	{
		return *TagCountMap.Find(Tag);
	}

	return INDEX_NONE;
}

void FInventoryItemEntry::SetStatValue(const FGameplayTag& Tag, int32 Value)
{
	TagCountMap.Add(Tag, Value);
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
	if (InArraySerializer.OwningInventory)
	{
		InArraySerializer.OwningInventory->OnGiveItem(*this);
	}
}

void FInventoryItemEntry::PostReplicatedChange(const FInventoryItemContainer& InArraySerializer)
{
}

FInventoryItemContainer::FInventoryItemContainer()
	: OwningInventory(nullptr)
{
}

FInventoryItemContainer::FInventoryItemContainer(AInventoryBase* InOwningInventory)
	: OwningInventory(InOwningInventory)
{
}

void FInventoryItemContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Index : RemovedIndices)
	{
		FInventoryItemEntry& Entry = Items[Index];
		Entry.LastObservedStackCount = 0;
	}
}

void FInventoryItemContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FInventoryItemEntry& Entry = Items[Index];
		Entry.LastObservedStackCount = Entry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
	}
}

void FInventoryItemContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Index : ChangedIndices)
	{
		FInventoryItemEntry& Entry = Items[Index];
		check(Entry.LastObservedStackCount != INDEX_NONE);
		Entry.LastObservedStackCount = Entry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
	}
}
