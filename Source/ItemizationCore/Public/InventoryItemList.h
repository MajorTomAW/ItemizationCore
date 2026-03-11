// Author: Tom Werner (dc: majort), 2026

#pragma once

#include "InventoryItemId.h"
#include "ItemInstanceBase.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventoryItemList.generated.h"

#define UE_API ITEMIZATIONCORE_API

struct FItemAndCount;
class UItemDefinitionBase;
class UItemInstanceBase;
class AInventoryBase;
struct FInventoryItemList;

/** A single entry in an inventory list. */
USTRUCT(BlueprintType)
struct FInventoryItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	friend struct FInventoryItemList;
	friend class AInventoryBase;

	UE_API FInventoryItemEntry();
	UE_API FInventoryItemEntry(const FInventoryItemEntry& Other);
	UE_API explicit FInventoryItemEntry(const UItemDefinitionBase* InItemDefinition, int32 InCount, UObject* InSourceObject = nullptr);
	UE_API explicit FInventoryItemEntry(const FItemAndCount& ItemAndCount, UObject* InSourceObject = nullptr);

	//~ Begin FFastArraySerializerItem Interface
	void PreReplicatedRemove(const FInventoryItemList& InArraySerializer);
	void PostReplicatedAdd(const FInventoryItemList& InArraySerializer);
	void PostReplicatedChange(const FInventoryItemList& InArraySerializer);
	//~ End FFastArraySerializerItem Interface

	/** Resets this item entry to an invalid state. */
	UE_API void Reset();

	/** Returns true if this item entry has a valid item definition and handle. */
	UE_API bool IsValid() const;

	/** Returns this item entry as a debug string. */
	UE_API FString GetDebugString() const;

	/** Returns the owning inventory of this item. */
	AInventoryBase* GetOwningInventory() const { return OwningInventory.Get(); }

	/** Sets the owning inventory of this item. */
	UE_API void SetOwningInventory(AInventoryBase* InOwningInventory);

	/** Returns the source object that gave us this item. */
	UObject* GetSourceObject() const { return SourceObject.Get(); }

	/** Returns the current stack size. */
	int32 GetStackSize() const { return StackSize; }

	/** Sets the stack size to the given value and marks it dirty in the owning inventory. */
	UE_API void SetStackSize(int32 NewStackSize);

	/** Sets the stack size to the given value without marking it dirty. */
	UE_API void SetStackSizeNoDirty(int32 NewStackSize);

	/** Sets the durability to the given value and marks it dirty in the owning inventory. */
	UE_API void SetDurability(float NewDurability);

	/** Gets the current durability. */
	float GetDurability() const { return Durability; }

	/** Returns the last stack size that was locally observed. */
	int32 GetLastObservedStackSize() const { return LastObservedStackSize; }

	/** Sets the last observed stack size. */
	void SetLastObservedStackSize(int32 NewLastObservedStackSize) { LastObservedStackSize = NewLastObservedStackSize; }

	/** Marks this item dirty. */
	UE_API void MarkItemDirty();

	/** Returns the item instance associated with this entry. */
	UE_API UItemInstanceBase* GetItemInstance() const { return ItemInstance; }
	template <class InstanceType UE_REQUIRES(std::is_base_of_v<UItemInstanceBase, InstanceType>)>
	InstanceType* GetItemInstance() const
	{
		return Cast<InstanceType>(GetItemInstance());
	}

	/** Sets  the item instance. */
	void SetItemInstance(UItemInstanceBase* NewItemInstance)
	{
		ItemInstance = NewItemInstance;
	}

	/** Returns the item id. */
	const FInventoryItemId& GetItemId() const { return ItemId; }
	FInventoryItemId& GetItemId_Ref() { return ItemId; }

	/** Returns the item definition. */
	const UItemDefinitionBase* GetItemDefinition() const { return ItemDefinition; }
	template <class ItemType>
	const ItemType* GetItemDefinition() const
	{
		return Cast<const ItemType>(GetItemDefinition());
	}

private:
	/** The unique id to this item for outside references. */
	UPROPERTY()
	FInventoryItemId ItemId;

	/** The item definition that this entry represents. */
	UPROPERTY()
	TObjectPtr<const UItemDefinitionBase> ItemDefinition;

	/** Replicated item instance. */
	UPROPERTY()
	TObjectPtr<UItemInstanceBase> ItemInstance;
	uint8 bWaitingOnItemInstance:1;

	/** Optional source object that granted this item */
	UPROPERTY()
	TWeakObjectPtr<UObject> SourceObject;


	/** Durability of this item. */
	UPROPERTY()
	float Durability;

	/** Current size of the item. */
	UPROPERTY()
	int32 StackSize;

	/** The last stack size that was locally observed. */
	UPROPERTY(NotReplicated)
	int32 LastObservedStackSize;

	/** Pending removal due to scope lock */
	UPROPERTY(NotReplicated)
	uint8 bPendingRemove:1;

	/** Reference to the owning inventory. */
	UPROPERTY(NotReplicated)
	TObjectPtr<AInventoryBase> OwningInventory;

public:
	UE_API FInventoryItemEntry& operator=(const FInventoryItemEntry& Other);
	UE_API FInventoryItemEntry& operator=(FInventoryItemEntry& Other);

	bool operator==(const FInventoryItemEntry& Other) const
	{
		return (ItemId.Get() == Other.ItemId.Get()) &&
			(StackSize == Other.StackSize) &&
			(ItemDefinition == Other.ItemDefinition);
	}

	bool operator!=(const FInventoryItemEntry& Other) const
	{
		return !operator==(Other);
	}

	bool operator==(const UItemInstanceBase* OtherInstance) const
	{
		return GetItemInstance() == OtherInstance;
	}

	bool operator==(const FInventoryItemId& OtherHandle) const
	{
		return ItemId.Get() == OtherHandle.Get();
	}

	bool operator==(const UItemDefinitionBase* OtherDefinition) const
	{
		return ItemDefinition == OtherDefinition;
	}

	bool operator>(const FInventoryItemEntry& Other) const
	{
		return StackSize > Other.StackSize;
	}
};

template<>
struct TStructOpsTypeTraits<FInventoryItemEntry> : TStructOpsTypeTraitsBase2<FInventoryItemEntry>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithNetSharedSerialization = true,
	};
};

/** An inventory list inside an inventory. */
USTRUCT(BlueprintType)
struct FInventoryItemList : public FFastArraySerializer
{
	GENERATED_BODY()
	friend struct FInventoryItemEntry;
	friend class AInventoryBase;

	UE_API FInventoryItemList();
	UE_API FInventoryItemList(AInventoryBase* InOwningInventory);

	/**
	 * Adds an item entry to this list.
	 * Will also generate a new item id and create a new item instance if none was already set.
	 * Calls NotifyItemAdded on the owning inventory
	 */
	UE_API FInventoryItemEntry& AddItemToList(FInventoryItemEntry ItemEntry);

	/**
	 * Removes an item from this list.
	 * Calls NotifyItemRemoved on  the owning inventory through OnRemoveItem.
	 *
	 * @param ItemId The id of the item to be removed
	 * @returns True, if we removed anything.
	 */
	UE_API bool RemoveItemFromList(const FInventoryItemId& ItemId);
	UE_API bool RemoveItemFromList(const UItemInstanceBase* ItemInstance);
	UE_API bool RemoveItemFromList(const FInventoryItemEntry& ItemEntry);

	/** Tries to find an FInventoryItemEntry by its id. */
	UE_API FInventoryItemEntry* FindItemEntryById(const FInventoryItemId& ItemId) const;

	/** Tries to find the first FInventoryItemEntry with matching item instance. Slower than with item id. */
	UE_API FInventoryItemEntry* FindFirstItemEntryByDefinition(const UItemDefinitionBase* ItemDefinition) const;

	/** Tries to find an item instance by its id. */
	UE_API UItemInstanceBase* FindItemInstanceById(const FInventoryItemId& ItemId) const;

	UE_API bool ContainsItem(const UItemInstanceBase* ItemInstance) const;
	UE_API bool ContainsItem(const FInventoryItemId& ItemId) const;

	//~ Begin FFastArraySerializer Interface
	UE_API void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	UE_API void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	UE_API void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryItemEntry, FInventoryItemList>(Items, DeltaParms, *this);
	}

	template <typename  Type, typename SerializerType>
	bool ShouldWriteFastArrayItem(const Type& Item, const bool bIsWritingOnClient)
	{
		if (bIsWritingOnClient)
		{
			return Item.ReplicationID != INDEX_NONE;
		}

		return true;
	}
	//~ End FFastArraySerializer Interface

	int32 Num() const { return Items.Num(); }
	const auto& operator[](int32 Index) const { return Items[Index]; }
	TArray<FInventoryItemEntry>::TConstIterator CreateConstIterator() const { return Items.CreateConstIterator(); }
	TArray<FInventoryItemEntry>::TIterator CreateIterator() { return Items.CreateIterator(); }
	friend TArray<FInventoryItemEntry>::TConstIterator begin(const FInventoryItemList& Array) { return Array.CreateConstIterator(); }
	friend TArray<FInventoryItemEntry>::TConstIterator end(const FInventoryItemList& Array) { return TArray<FInventoryItemEntry>::TConstIterator(Array.Items, Array.Items.Num()); }
	friend TArray<FInventoryItemEntry>::TIterator begin(FInventoryItemList& Array) { return Array.CreateIterator(); }
	friend TArray<FInventoryItemEntry>::TIterator end(FInventoryItemList& Array) { return TArray<FInventoryItemEntry>::TIterator(Array.Items, Array.Items.Num()); }

private:
	/** List of item entries in this inventory. */
	UPROPERTY()
	TArray<FInventoryItemEntry> Items;

	/** Actor that owns this inventory. */
	UPROPERTY(NotReplicated)
	TObjectPtr<AInventoryBase> OwningInventory;

	/** Fast lookup for item instances by Id. */
	TMap<FInventoryItemId, TWeakObjectPtr<UItemInstanceBase>> ItemInstanceLookupTable;
};

template<>
struct TStructOpsTypeTraits<FInventoryItemList> : TStructOpsTypeTraitsBase2<FInventoryItemList>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithNetSharedSerialization = true,
	};
};

/** Used to lock the inventory list to prevent it from changing size during iteration. */
struct FScopedInventoryLock
{
	UE_API FScopedInventoryLock(AInventoryBase& InInventory);
	UE_API ~FScopedInventoryLock();

private:
	AInventoryBase& Inventory;
};

#undef UE_API
