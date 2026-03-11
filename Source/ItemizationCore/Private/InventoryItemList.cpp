// Author: Tom Werner (dc: majort), 2026


#include "InventoryItemList.h"

#include "InventoryBase.h"
#include "ItemInstanceBase.h"
#include "ItemizationCoreLogChannels.h"
#include "Items/ItemAndCount.h"
#include "Items/ItemDefinitionBase.h"

FInventoryItemEntry::FInventoryItemEntry()
	: ItemInstance(nullptr)
	, bWaitingOnItemInstance(true)
	, Durability(0.f)
	, StackSize(INDEX_NONE)
	, LastObservedStackSize(INDEX_NONE)
	, bPendingRemove(false)
	, OwningInventory(nullptr)
{
	ItemId.GenerateNewId();
}

FInventoryItemEntry::FInventoryItemEntry(const FInventoryItemEntry& Other)
{
	Durability = Other.Durability;
	StackSize = Other.StackSize;
	LastObservedStackSize = Other.LastObservedStackSize;
	ItemDefinition = Other.ItemDefinition;
	SourceObject = Other.SourceObject;
	bPendingRemove = Other.bPendingRemove;
	bWaitingOnItemInstance = Other.bWaitingOnItemInstance || !::IsValid(ItemInstance);
	OwningInventory = Other.OwningInventory;
}

FInventoryItemEntry::FInventoryItemEntry(
	const UItemDefinitionBase* InItemDefinition,
	int32 InCount,
	UObject* InSourceObject)
		: ItemDefinition(InItemDefinition)
		, ItemInstance(nullptr)
		, bWaitingOnItemInstance(true)
		, SourceObject(InSourceObject)
		, Durability(0.f)
		, bPendingRemove(false)
{
	// Sometimes a negative value can be passed in,
	// we treat it as zero
	if (InCount < 0)
	{
		StackSize = 0;
	}
	else
	{
		StackSize = InCount;
	}
	LastObservedStackSize = StackSize;

	// Reset the parent inventory
	OwningInventory = nullptr;
}

FInventoryItemEntry::FInventoryItemEntry(const FItemAndCount& ItemAndCount, UObject* InSourceObject)
	: FInventoryItemEntry(ItemAndCount.ItemDefinition, ItemAndCount.StackSize, InSourceObject)
{
}

void FInventoryItemEntry::PreReplicatedRemove(const FInventoryItemList& InArraySerializer)
{
	if (InArraySerializer.OwningInventory)
	{
		UE_LOG(LogItemization, Verbose, NET_TEXT(InArraySerializer.OwningInventory, "%s InventoryItemEntry removed (Non-Auth): [%s] %s.",
			*GetNameSafe(InArraySerializer.OwningInventory->GetOwner()), *ItemId.ToString(), *GetNameSafe(ItemInstance)))

		FScopedInventoryLock(*InArraySerializer.OwningInventory);
		InArraySerializer.OwningInventory->OnRemoveItem(*this);
	}
}

void FInventoryItemEntry::PostReplicatedAdd(const FInventoryItemList& InArraySerializer)
{
	if (!InArraySerializer.OwningInventory)
	{
		return;
	}

	if (!::IsValid(ItemInstance))
	{
		UE_LOG(LogItemization, Warning, NET_TEXT(InArraySerializer.OwningInventory, "%s InventoryItemEntry added (Non-Auth) [%s] %s. But ItemInstance hasn't been resolved yet. "
			"Setting bWaitingOnItemInstance to true.",
			*GetNameSafe(InArraySerializer.OwningInventory->GetOwner()), *ItemId.ToString(), *GetNameSafe(ItemInstance)))

		bWaitingOnItemInstance = true;

	}
	else
	{
		UE_LOG(LogItemization, Verbose, NET_TEXT(InArraySerializer.OwningInventory, "%s InventoryItemEntry added (Non-Auth): [%s] %s.",
			*GetNameSafe(InArraySerializer.OwningInventory->GetOwner()), *ItemId.ToString(), *GetNameSafe(ItemInstance)))

		InArraySerializer.OwningInventory->OnGiveItem(*this);
	}
}

void FInventoryItemEntry::PostReplicatedChange(const FInventoryItemList& InArraySerializer)
{
	if (!InArraySerializer.OwningInventory)
	{
		return;
	}

	if (bWaitingOnItemInstance && ::IsValid(ItemInstance))
	{
		UE_LOG(LogItemization, Log, NET_TEXT(InArraySerializer.OwningInventory, "%s InventoryItemEntry changed (Non-Auth) [%s] %s, "
			"but we were waiting for an item instance which has now been resolved, calling OnGiveItem().",
			*GetNameSafe(InArraySerializer.OwningInventory->GetOwner()), *ItemId.ToString(), *GetNameSafe(ItemInstance)))

		bWaitingOnItemInstance = false;
		InArraySerializer.OwningInventory->OnGiveItem(*this);
	}
	else if (::IsValid(ItemInstance))
	{
		UE_LOG(LogItemization, Verbose, NET_TEXT(InArraySerializer.OwningInventory, "%s InventoryItemEntry changed (Non-Auth): [%s] %s.",
			*GetNameSafe(InArraySerializer.OwningInventory->GetOwner()), *ItemId.ToString(), *GetNameSafe(ItemInstance)))

		// call the notify change directly
		// I don't want to go through the same process as OnGiveItem/OnRemoveItem to make a
		// OnItemChanged which I would have to call server-side each time i make modifications to a single item entry.
		InArraySerializer.OwningInventory->NotifyItemChanged(*this, LastObservedStackSize, StackSize);
	}
	else
	{
		bWaitingOnItemInstance = true;
	}
}

void FInventoryItemEntry::Reset()
{
	ItemInstance = nullptr;
	ItemDefinition = nullptr;
	SourceObject = nullptr;
	StackSize = INDEX_NONE;
	LastObservedStackSize = INDEX_NONE;
	ItemId.Reset();
	bPendingRemove = false;
}

bool FInventoryItemEntry::IsValid() const
{
	return ::IsValid(ItemInstance);
}

FString FInventoryItemEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s [%s]"), *GetNameSafe(GetItemDefinition()), *ItemId.ToString());
}

void FInventoryItemEntry::SetOwningInventory(AInventoryBase* InOwningInventory)
{
	OwningInventory = InOwningInventory;
}

void FInventoryItemEntry::SetStackSize(int32 NewStackSize)
{
	// Make sure we don't go into the negative
	NewStackSize = FMath::Max(NewStackSize, 0);

	// Only update if we need to
	if (NewStackSize != StackSize)
	{
		StackSize = NewStackSize;
		MarkItemDirty();
	}
}

void FInventoryItemEntry::SetStackSizeNoDirty(int32 NewStackSize)
{
	// Make sure we don't go into the negative
	StackSize = FMath::Max(NewStackSize, 0);
}

void FInventoryItemEntry::SetDurability(float NewDurability)
{
	// Make sure we don't go into negative
	NewDurability = FMath::Max(NewDurability, 0);

	if (NewDurability != Durability)
	{
		Durability = NewDurability;
		MarkItemDirty();
	}
}

void FInventoryItemEntry::MarkItemDirty()
{
	if (::IsValid(OwningInventory))
	{
		OwningInventory->MarkItemEntryDirty(*this, true);
	}
}

FInventoryItemEntry& FInventoryItemEntry::operator=(const FInventoryItemEntry& Other)
{
	StackSize = Other.StackSize;
	LastObservedStackSize = Other.LastObservedStackSize;
	ItemDefinition = Other.ItemDefinition;
	SourceObject = Other.SourceObject;
	OwningInventory = Other.OwningInventory;
	return *this;
}

FInventoryItemEntry& FInventoryItemEntry::operator=(FInventoryItemEntry& Other)
{
	StackSize = Other.StackSize;
	LastObservedStackSize = Other.LastObservedStackSize;
	ItemDefinition = Other.ItemDefinition;
	SourceObject = Other.SourceObject;
	OwningInventory = Other.OwningInventory;
	return *this;
}

FInventoryItemList::FInventoryItemList()
	: OwningInventory(nullptr)
{
}

FInventoryItemList::FInventoryItemList(AInventoryBase* InInventoryBase)
	: OwningInventory(InInventoryBase)
{
}

FInventoryItemEntry* FInventoryItemList::FindItemEntryById(const FInventoryItemId& ItemId) const
{
	return const_cast<FInventoryItemEntry*>(Items.FindByKey(ItemId));
}

FInventoryItemEntry* FInventoryItemList::FindFirstItemEntryByDefinition(const UItemDefinitionBase* ItemDefinition) const
{
	for (const FInventoryItemEntry& ItemEntry : Items)
	{
		if (ItemEntry.GetItemDefinition() == ItemDefinition)
		{
			return const_cast<FInventoryItemEntry*>(&ItemEntry);
		}
	}

	return nullptr;
}

UItemInstanceBase* FInventoryItemList::FindItemInstanceById(const FInventoryItemId& ItemId) const
{
	if (const auto* Found = ItemInstanceLookupTable.Find(ItemId))
	{
		if (Found->IsValid())
		{
			return Found->Get();
		}
	}

	// Not found, but might be due to replication timing, check the item entry
	if (const auto* Entry = FindItemEntryById(ItemId))
	{
		return Entry->GetItemInstance();
	}

	return nullptr;
}

bool FInventoryItemList::ContainsItem(const UItemInstanceBase* ItemInstance) const
{
	return Items.Contains(ItemInstance);
}

bool FInventoryItemList::ContainsItem(const FInventoryItemId& ItemId) const
{
	return ItemInstanceLookupTable.Contains(ItemId);
}

FInventoryItemEntry& FInventoryItemList::AddItemToList(FInventoryItemEntry ItemEntry)
{
	if (OwningInventory->IsInventoryLocked())
	{
		UE_LOG(LogItemization, Verbose, NET_TEXT(OwningInventory, "%s AddItemToList %s delayed (ScopeLocked)",
			*GetNameSafe(OwningInventory->GetOwner()), *ItemEntry.GetDebugString()))
		OwningInventory->AddPendingItemAdd(ItemEntry);
		return ItemEntry;
	}

	// Add it
	FScopedInventoryLock ScopeLock(*OwningInventory);
	FInventoryItemEntry& NewEntry = Items.Emplace_GetRef(ItemEntry);
	NewEntry.SetOwningInventory(OwningInventory);

	// Generate a new id
	if (!NewEntry.GetItemId().IsValid())
	{
		NewEntry.GetItemId_Ref().GenerateNewId();
	}

	// Create the item instance
	if (NewEntry.GetItemInstance() == nullptr)
	{
		OwningInventory->CreateNewItemInstance(NewEntry);
	}

	// Add it to the lookup table
	ItemInstanceLookupTable.Add(NewEntry.GetItemId(), NewEntry.GetItemInstance());
	MarkItemDirty(NewEntry);

	// Notify the inventory
	OwningInventory->OnGiveItem(NewEntry);

	return NewEntry;
}

bool FInventoryItemList::RemoveItemFromList(const FInventoryItemId& ItemId)
{
	if (!ensure(ItemId.IsValid()))
	{
		return false;
	}

	if (OwningInventory->IsInventoryLocked())
	{
		UE_LOG(LogItemization, Verbose, NET_TEXT(OwningInventory, "%s RemoveItemFromList %s delayed (ScopeLocked)",
			*GetNameSafe(OwningInventory->GetOwner()), *ItemId.ToString()))

		// Mark as pending remove
		for (auto It = Items.CreateIterator(); It; ++It)
		{
			FInventoryItemEntry& Entry = *It;
			if (Entry.GetItemId() == ItemId)
			{
				Entry.bPendingRemove = true;
				break;
			}
		}

		OwningInventory->AddPendingItemRemove(ItemId);
		return true;
	}

	for (auto It = Items.CreateIterator(); It; ++It)
	{
		FInventoryItemEntry& Entry = *It;
		if (Entry.GetItemId() == ItemId)
		{
			// Notify inventory
			OwningInventory->OnRemoveItem(Entry);

			It.RemoveCurrent();
			ItemInstanceLookupTable.Remove(Entry.GetItemId());

			MarkArrayDirty();
			return true;
		}
	}

	return false;
}

bool FInventoryItemList::RemoveItemFromList(const UItemInstanceBase* ItemInstance)
{
	return RemoveItemFromList(ItemInstance->GetItemId());
}

bool FInventoryItemList::RemoveItemFromList(const FInventoryItemEntry& ItemEntry)
{
	return RemoveItemFromList(ItemEntry.GetItemId());
}

void FInventoryItemList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Index : RemovedIndices)
	{
		FInventoryItemEntry& Entry = Items[Index];
		Entry.LastObservedStackSize = 0;
	}
}

void FInventoryItemList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FInventoryItemEntry& Entry = Items[Index];
		Entry.SetOwningInventory(OwningInventory);
		Entry.LastObservedStackSize = Entry.GetStackSize();

		// Add to lookup table
		ItemInstanceLookupTable.Add(Entry.GetItemId(), Entry.GetItemInstance());
	}
}

void FInventoryItemList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Index : ChangedIndices)
	{
		FInventoryItemEntry& Entry = Items[Index];
		check(Entry.LastObservedStackSize != INDEX_NONE)
		Entry.LastObservedStackSize = Entry.GetStackSize();

		// Update lookup table
		if (ItemInstanceLookupTable[Entry.GetItemId()] != Entry.GetItemInstance())
		{
			ItemInstanceLookupTable[Entry.GetItemId()] = Entry.GetItemInstance();
		}
	}
}

FScopedInventoryLock::FScopedInventoryLock(AInventoryBase& InInventory)
	: Inventory(InInventory)
{
	Inventory.IncrementInventoryLock();
}

FScopedInventoryLock::~FScopedInventoryLock()
{
	Inventory.DecrementInventoryLock();
}
