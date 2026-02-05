// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "InventoryItemId.h"
#include "ItemizationCoreMacros.h"
#include "ItemizationCoreTags.h"
#include "Data/ItemComponentDataList.h"
#include "Net/Serialization/FastArraySerializer.h"


#include "InventoryItemEntry.generated.h"

struct FItemAndCount;
class IInventoryItemInstanceInterface;
class AInventoryBase;
class UItemDefinitionBase;
class UInventoryItemInstance;
class UInventoryComponent;

#define UE_API ITEMIZATIONCORE_API

/** Fast array serializer item for a single item entry in an inventory. */
USTRUCT(BlueprintType)
struct FInventoryItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	friend struct FInventoryItemList;
	friend class UInventoryComponent;
	friend class AInventoryBase;

public:
	UE_API FInventoryItemEntry();
	UE_API FInventoryItemEntry(const FInventoryItemEntry& Other);
	UE_API explicit FInventoryItemEntry(const UItemDefinitionBase* InItemDefinition, int32 InCount, UObject* InSourceObject = nullptr);
	UE_API explicit FInventoryItemEntry(const FItemAndCount& ItemAndCount, UObject* InSourceObject = nullptr);

	/** Returns this item entry as a debug string. */
	UE_API FString GetDebugString() const;

	/** Returns this item entry's item definitions' name. */
	UE_API FText GetItemName(bool bUsePlural = false) const;

	/** Prints out all stats associated with this item entry. */
	UE_API void DebugPrintStats() const;

	/** Resets this item entry to an invalid state. */
	UE_API void Reset();

	/** Returns true if this item entry has a valid item definition and handle. */
	UE_API bool IsValid() const;

	/** Returns the item instance associated with this entry. */
	UE_API TScriptInterface<IInventoryItemInstanceInterface> GetItemInstance() const;
	UE_API void SetReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& InInstance);
	UE_API void SetNonReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& InInstance);

	/** Returns the owning inventory of this item. */
	AInventoryBase* GetOwningInventory() const { return OwningInventory.Get(); }

	/** Sets the owning inventory of this item. */
	UE_API void SetOwningInventory(AInventoryBase* InOwningInventory);

	/*/** Returns a stat integer associated with the given tag. #1#
	int32 GetStatValue(const FGameplayTag& Tag) const;

	/** Sets the stat integer associated with the given tag. #1#
	void SetStatValue(const FGameplayTag& Tag, int32 Value);

	/** Returns all stat tags associated with this item. #1#
	const TArray<FGameplayTagStack>& GetAllStats() const { return TagCountMap.GetGameplayTagStackArray(); }

	/** Returns all stats as the raw tag count map. #1#
	const FGameplayTagStackContainer& GetStatCountMap() const { return TagCountMap; }*/

	/** Returns the item id. */
	const FInventoryItemId& GetItemId() const { return ItemId; }
	FInventoryItemId& GetItemId_Ref() { return ItemId; }

	/** Returns the item definition. */
	const UItemDefinitionBase* GetItemDefinition() const { return ItemDefinition; }

	/** Returns the source object that gave us this item. */
	UObject* GetSourceObject() const { return SourceObject.Get(); }

	/** Returns the current stack size. */
	int32 GetStackSize() const { return StackSize; }

	/** Sets the stack size to the given value and marks it dirty in the owning inventory. */
	UE_API void SetStackSize(int32 NewStackSize);

	/** Sets the stack size to the given value without marking it dirty. */
	UE_API void SetStackSizeNoDirty(int32 NewStackSize);

	/** Returns the last stack size that was locally observed. */
	int32 GetLastObservedStackSize() const { return LastObservedStackSize; }

	/** Sets the last observed stack size. */
	void SetLastObservedStackSize(int32 NewLastObservedStackSize) { LastObservedStackSize = NewLastObservedStackSize; }

	/** Marks this item dirty. */
	UE_API void MarkItemDirty();

	//~ Begin FFastArraySerializerItem Interface
	void PreReplicatedRemove(const FInventoryItemList& InArraySerializer);
	void PostReplicatedAdd(const FInventoryItemList& InArraySerializer);
	void PostReplicatedChange(const FInventoryItemList& InArraySerializer);
	//~ End FFastArraySerializerItem Interface

private:
	/** The unique id to this item for outside references. */
	UPROPERTY()
	FInventoryItemId ItemId;

	/** The item definition that this entry represents. */
	UPROPERTY()
	TObjectPtr<const UItemDefinitionBase> ItemDefinition;

	/** Optional source object that granted this item */
	UPROPERTY()
	TWeakObjectPtr<UObject> SourceObject;

	/** Current size of the item. */
	UPROPERTY()
	int32 StackSize;

	/** The last stack size that was locally observed. */
	UPROPERTY(NotReplicated)
	int32 LastObservedStackSize;

	/** Pending removal due to scope lock */
	UPROPERTY(NotReplicated)
	uint8 bPendingRemove:1;

	/** Flag that indicates whether this entry has been altered from outside. */
	UPROPERTY(NotReplicated)
	bool bIsDirty;

	/** Reference to the owning inventory. */
	UPROPERTY(NotReplicated)
	TWeakObjectPtr<AInventoryBase> OwningInventory;

protected:
	/** Replicated item instance */
	UPROPERTY()
	TObjectPtr<UObject> ReplicatedInstance;

	/** Non-replicated item instance */
	UPROPERTY(NotReplicated)
	TObjectPtr<UObject> NonReplicatedInstance;

	/**@TODO: Authority-only list of stat tags mapped to stat integer value.*/

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

	bool operator==(const UObject* OtherInstance) const
	{
		return GetItemInstance().GetObject() == OtherInstance;
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

/** Fast array serializer for a list of item entries in an inventory. */
USTRUCT(BlueprintType)
struct FInventoryItemList : public FFastArraySerializer
{
	GENERATED_BODY()
	friend struct FInventoryItemEntry;
	friend class UInventoryComponent;
	friend class AInventoryBase;

public:
	UE_API FInventoryItemList();
	UE_API FInventoryItemList(AInventoryBase* InOwningInventory);

	/** Tries to find an FInventoryItemEntry by its id. Fast lookup. */
	UE_API FInventoryItemEntry* FindItemEntryById(const FInventoryItemId& ItemId) const;

	/** Tries to find the first FInventoryItemEntry with matching item instance. Slower than with item id. */
	UE_API FInventoryItemEntry* FindFirstItemEntryByDefinition(const UItemDefinitionBase* ItemDefinition) const;

	/** Tries to find an item instance by its id. */
	UE_API UObject* FindItemInstanceById(const FInventoryItemId& ItemId) const;

	/**
	 * Adds an item entry to this list.
	 * Will also generate a new item id and create a new item instance if none was already set.
	 * Calls NotifyItemAdded on the owning inventory
	 */
	UE_API FInventoryItemEntry& AddItemToList(FInventoryItemEntry ItemEntry);

	/**
	 * Removes an item from the list using its Id.
	 * Calls NotifyItemRemoved on the owning inventory.
	 */
	UE_API bool RemoveItemFromList(FInventoryItemId ItemId);

	/**
	 * Removes an item from the list using the instance.
	 * Calls NotifyItemRemoved on the owning inventory.
	 */
	UE_API bool RemoveItemFromList(TScriptInterface<IInventoryItemInstanceInterface> ItemInstance);

	/**
	 * Removes an item from the list using the entry.
	 * Calls NotifyItemRemoved on the owning inventory.
	 */
	UE_API bool RemoveItemFromList(const FInventoryItemEntry& ItemEntry);

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
			return Item.ReplicationID != INDEX_NONE || Item.bIsDirty;
		}

		return true;
	}
	//~ End FFastArraySerializer Interface

	/** TArray accessors for this container. */
	CREATE_ARRAY_SERIALIZER_TARRAY_ACCESSORS(FInventoryItemList, FInventoryItemEntry, Items);

protected:
	/** List of item entries in this inventory. */
	UPROPERTY()
	TArray<FInventoryItemEntry> Items;

	/** The Inventory class that owns this list. */
	UPROPERTY(NotReplicated)
	TObjectPtr<AInventoryBase> OwningInventory;

	/** Fast lookup for item instances by Id. */
	TMap<FInventoryItemId, TObjectPtr<UObject>> ItemInstanceMap;
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

#undef UE_API
