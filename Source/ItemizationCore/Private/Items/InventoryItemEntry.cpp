// Author: Tom Werner (MajorT), 2025


#include "Items/InventoryItemEntry.h"

#include "ItemizationCoreLogChannels.h"
#include "Inventory/InventoryBase.h"
#include "Items/InventoryItemInstance.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FInventoryItemEntry

FInventoryItemEntry::FInventoryItemEntry()
	: LastObservedStackCount(INDEX_NONE)
	, bPendingRemove(false)
{
}

FInventoryItemEntry::FInventoryItemEntry(
	UItemDefinitionBase* InItemDefinition,
	UObject* InSourceObject,
	int32 InCount)
		: ItemDefinition(InItemDefinition)
		, SourceObject(InSourceObject)
		, LastObservedStackCount(InCount)
		, bPendingRemove(false)
{
	SetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize, InCount);
}

FString FInventoryItemEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s [%s]"), *GetNameSafe(GetItemInstance().GetObject()), *ItemHandle.ToString());
}

FString FInventoryItemEntry::GetItemName() const
{
	return GetNameSafe(ItemDefinition);
}

void FInventoryItemEntry::DebugPrintStats() const
{
#if ENABLE_DRAW_DEBUG
	for (const FGameplayTagStack& Stack : GetAllStats())
	{
		ITEMIZATION_LOG("\t%s",*Stack.ToString());
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
	LastObservedStackCount = INDEX_NONE;
	ItemHandle.Reset();
	bPendingRemove = false;
	TagCountMap.Reset();
}

TScriptInterface<IInventoryItemInstanceInterface> FInventoryItemEntry::GetItemInstance() const
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

void FInventoryItemEntry::SetReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& InInstance)
{
	checkf(NonReplicatedInstance == nullptr,
		TEXT("You cannot set the replicated instance if a non-replicated instance [%s] already exists!"),
		*GetNameSafe(NonReplicatedInstance));

	ReplicatedInstance = InInstance.GetObject();
}

void FInventoryItemEntry::SetNonReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& InInstance)
{
	checkf(ReplicatedInstance == nullptr,
		TEXT("You cannot set the non-replicated instance if a replicated instance [%s] already exists!"),
		*GetNameSafe(ReplicatedInstance));

	NonReplicatedInstance = InInstance.GetObject();
}

int32 FInventoryItemEntry::GetStatValue(const FGameplayTag& Tag) const
{
	if (TagCountMap.HasTag(Tag))
	{
		return TagCountMap.GetStackCount(Tag);
	}

	return INDEX_NONE;
}

void FInventoryItemEntry::SetStatValue(const FGameplayTag& Tag, int32 Value)
{
	TagCountMap.SetStackCount(Tag, Value);
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FInventoryItemContainer

FInventoryItemContainer::FInventoryItemContainer()
	: OwningInventory(nullptr)
{
}

FInventoryItemContainer::FInventoryItemContainer(AInventoryBase* InOwningInventory)
	: OwningInventory(InOwningInventory)
{
}

FInventoryItemEntry* FInventoryItemContainer::FindItemEntryByHandle(const FInventoryItemHandle& ItemHandle) const
{
	for (auto& Entry : Items)
	{
		// Skip pending removals
		if (Entry.bPendingRemove)
		{
			continue;
		}

		if (Entry != ItemHandle)
		{
			continue;
		}

		return const_cast<FInventoryItemEntry*>(&Entry);
	}

	return nullptr;
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
