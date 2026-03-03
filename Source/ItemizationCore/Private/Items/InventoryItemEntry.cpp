// Author: Tom Werner (MajorT), 2025


#include "Items/InventoryItemEntry.h"

#include "ItemizationCoreLogChannels.h"
#include "Inventory/InventoryBase.h"
#include "Items/InventoryItemInstance.h"
#include "Items/ItemAndCount.h"
#include "Items/ItemDefinitionBase.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FInventoryItemEntry

FInventoryItemEntry::FInventoryItemEntry()
	: StackSize(INDEX_NONE)
	, LastObservedStackSize(INDEX_NONE)
	, bPendingRemove(false)
	, bIsDirty(false)
{
}

FInventoryItemEntry::FInventoryItemEntry(const FInventoryItemEntry& Other)
{
	StackSize = Other.StackSize;
	LastObservedStackSize = Other.LastObservedStackSize;
	ItemDefinition = Other.ItemDefinition;
	SourceObject = Other.SourceObject;
	OwningInventory = Other.OwningInventory;
	bPendingRemove = false;
	bIsDirty = true;
}

FInventoryItemEntry::FInventoryItemEntry(const UItemDefinitionBase* InItemDefinition, int32 InCount, UObject* InSourceObject)
	: ItemDefinition(InItemDefinition)
	, SourceObject(InSourceObject)
	, bPendingRemove(false)
	, bIsDirty(false)
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

FString FInventoryItemEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s [%s]"), *GetNameSafe(GetItemDefinition()), *ItemId.ToString());
}

FText FInventoryItemEntry::GetItemName(bool bUsePlural) const
{
	if (::IsValid(ItemDefinition))
	{
		return ItemDefinition->GetItemName(bUsePlural);
	}

	return FText::GetEmpty();
}

void FInventoryItemEntry::DebugPrintStats() const
{
#if ENABLE_DRAW_DEBUG
	FStringBuilderBase StringBuilder;
	StringBuilder.Append(TEXT("Stack Count: %d"), StackSize);

	/*for (const FGameplayTagStack& Stack : GetAllStats())
	{
		ITEMIZATION_LOG("\t%s",*Stack.ToString());
	}*/
#endif
}

void FInventoryItemEntry::Reset()
{
	ReplicatedInstance = nullptr;
	NonReplicatedInstance = nullptr;
	ItemDefinition = nullptr;
	SourceObject = nullptr;
	StackSize = INDEX_NONE;
	LastObservedStackSize = INDEX_NONE;
	ItemId.Reset();
	bPendingRemove = false;
}

bool FInventoryItemEntry::IsValid() const
{
	return ::IsValid(ItemDefinition) && ItemId.IsValid();
}

TScriptInterface<IInventoryItemInstanceInterface> FInventoryItemEntry::GetItemInstance() const
{
	if (::IsValid(ReplicatedInstance))
	{
		return ReplicatedInstance;
	}

	if (::IsValid(NonReplicatedInstance))
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

	checkf(InInstance->GetIsReplicated() == true,
		TEXT("Attempted to add NON REPLICATED instance '%s' as REPLICATED inside FInventoryItemEntry. Not allowed."),
		*GetNameSafe(InInstance.GetObject()))

	ReplicatedInstance = InInstance.GetObject();
}

void FInventoryItemEntry::SetNonReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& InInstance)
{
	checkf(ReplicatedInstance == nullptr,
		TEXT("You cannot set the non-replicated instance if a replicated instance [%s] already exists!"),
		*GetNameSafe(ReplicatedInstance));

	checkf(InInstance->GetIsReplicated() == false,
		TEXT("Attempted to add REPLICATED instance '%s' as NON REPLICATED inside FInventoryItemEntry. Not allowed."),
		*GetNameSafe(InInstance.GetObject()))

	NonReplicatedInstance = InInstance.GetObject();
}

void FInventoryItemEntry::SetOwningInventory(AInventoryBase* InOwningInventory)
{
	OwningInventory = InOwningInventory;
}

/*int32 FInventoryItemEntry::GetStatValue(const FGameplayTag& Tag) const
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
}*/

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

void FInventoryItemEntry::MarkItemDirty()
{
	if (OwningInventory.IsValid())
	{
		OwningInventory->MarkItemEntryDirty(*this, true);
	}

	bIsDirty = true;
}

void FInventoryItemEntry::PreReplicatedRemove(const FInventoryItemList& InArraySerializer)
{
	if (InArraySerializer.OwningInventory)
	{
		InArraySerializer.OwningInventory->OnRemoveItem(*this);
	}
}

void FInventoryItemEntry::PostReplicatedAdd(const FInventoryItemList& InArraySerializer)
{
	if (InArraySerializer.OwningInventory)
	{
		InArraySerializer.OwningInventory->OnGiveItem(*this);
	}
}

void FInventoryItemEntry::PostReplicatedChange(const FInventoryItemList& InArraySerializer)
{
	if (InArraySerializer.OwningInventory)
	{
		// call the notify change directly
		// I don't want to go through the same process as OnGiveItem/OnRemoveItem to make a
		// OnItemChanged which I would have to call server-side each time i make modifications to a single item entry.
		InArraySerializer.OwningInventory->NotifyItemChanged(*this, LastObservedStackSize, StackSize);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FInventoryItemList

FInventoryItemList::FInventoryItemList()
	: OwningInventory(nullptr)
{
}

FInventoryItemList::FInventoryItemList(AInventoryBase* InOwningInventory)
	: OwningInventory(InOwningInventory)
{
}

FInventoryItemEntry* FInventoryItemList::FindItemEntryById(const FInventoryItemId& ItemId) const
{
	for (const FInventoryItemEntry& ItemEntry : Items)
	{
		if (ItemEntry.GetItemId() == ItemId)
		{
			return const_cast<FInventoryItemEntry*>(&ItemEntry);
		}
	}

	return nullptr;
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

UObject* FInventoryItemList::FindItemInstanceById(
	const FInventoryItemId& ItemId) const
{
	if (const TObjectPtr<UObject>* Instance = ItemInstanceMap.Find(ItemId))
	{
		return *Instance;
	}

	return nullptr;
}

FInventoryItemEntry& FInventoryItemList::AddItemToList(FInventoryItemEntry ItemEntry)
{
	FInventoryItemEntry& NewEntry = Items.Emplace_GetRef(ItemEntry);
	NewEntry.SetOwningInventory(OwningInventory);

	// Generate a new item id if required
	if (!NewEntry.GetItemId().IsValid())
	{
		NewEntry.GetItemId_Ref().GenerateNewId();
	}

	// Create a new item instance if required
	if (NewEntry.GetItemInstance() == nullptr && OwningInventory->ShouldCreateNewItemInstance(NewEntry))
	{
		OwningInventory->CreateNewItemInstance(NewEntry);

		if (ensure(NewEntry.GetItemInstance()))
		{
			// Notify the item instance about it being added to the inventory
			NewEntry.GetItemInstance()->OnAddedToInventory(NewEntry, OwningInventory->InventoryHandle);
		}
	}

	// Also notify the item data
	/*for (const FItemComponentData* ItemData : ItemEntry.GetItemDefinition()->GetDataList())
	{
		if (ItemData != nullptr)
		{
			ItemData->OnItemGiven(ItemEntry, OwningInventory->InventoryHandle);
		}
	}*/

	// Add it to the lookup maps
	ItemInstanceMap.Add(NewEntry.GetItemId(), NewEntry.GetItemInstance().GetObject());

	MarkItemDirty(NewEntry);

	// Notify the inventory
	OwningInventory->OnGiveItem(NewEntry);

	return NewEntry;
}

bool FInventoryItemList::RemoveItemFromList(FInventoryItemId ItemId)
{
	if (!ItemId.IsValid())
	{
		return false;
	}

	for (auto It = Items.CreateIterator(); It; ++It)
	{
		FInventoryItemEntry& ItemEntry = *It;
		if (ItemEntry.GetItemId() == ItemId)
		{
			// Notify the inventory
			OwningInventory->OnRemoveItem(ItemEntry);

			It.RemoveCurrent();
			ItemInstanceMap.Remove(ItemId);

			MarkArrayDirty();
			return true;
		}
	}

	return false;
}

bool FInventoryItemList::RemoveItemFromList(TScriptInterface<IInventoryItemInstanceInterface> ItemInstance)
{
	if (ItemInstance->GetItemEntry() == nullptr)
	{
		return false;
	}

	return RemoveItemFromList(ItemInstance->GetItemEntry()->GetItemId());
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

		// Remove from maps
		ItemInstanceMap.Remove(Entry.GetItemId());
	}
}

void FInventoryItemList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FInventoryItemEntry& Entry = Items[Index];
		Entry.SetOwningInventory(OwningInventory);
		Entry.LastObservedStackSize = Entry.StackSize;

		// Add to maps
		ItemInstanceMap.Add(Entry.GetItemId(), Entry.GetItemInstance().GetObject());
	}
}

void FInventoryItemList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Index : ChangedIndices)
	{
		FInventoryItemEntry& Entry = Items[Index];
		check(Entry.LastObservedStackSize != INDEX_NONE);
		Entry.LastObservedStackSize = Entry.StackSize;

		// Update maps
		ItemInstanceMap[Entry.GetItemId()] = Entry.GetItemInstance().GetObject();
	}
}
