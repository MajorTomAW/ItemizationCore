// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryItemHandle.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventorySlotHandle.h"
#include "ItemizationCoreHelpers.h"
#include "Items/InventoryItemInstance.h"
#include "Items/ItemComponentDataList.h"

#include "InventoryItemEntry.generated.h"

class UItemDefinition;
class UInventoryManager;
class UInventoryItemInstance;
class AInventoryBase;

/** Fast array serializer for a single item entry in an inventory. */
USTRUCT(BlueprintType)
struct FInventoryItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	friend struct FInventoryItemContainer;
	friend class AInventoryBase;

	FInventoryItemEntry();
	FInventoryItemEntry(UItemDefinition* InItemDefinition, int32 InStackCount, UObject* InSourceObject = nullptr);

public:
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	FInventoryItemEntry(const FInventoryItemEntry&) = default;
	FInventoryItemEntry(FInventoryItemEntry&&) = default;
	FInventoryItemEntry& operator=(const FInventoryItemEntry&) = default;
	FInventoryItemEntry& operator=(FInventoryItemEntry&&) = default;
	~FInventoryItemEntry() = default;
PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
	/** The unique handle to this item for outside references. */
	UPROPERTY()
	FInventoryItemHandle ItemHandle;

	/** The item instance that this entry is representing. */
	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> Instance;

	/** The item definition asset that this entry is representing. */
	UPROPERTY()
	TObjectPtr<UItemDefinition> Definition;
	
	/**
	 * Object that this item was given from.
	 * Can be an actor or a static object.
	 * Useful to find previous ownership or to find out where this item came from.
	 */
	UPROPERTY()
	TWeakObjectPtr<UObject> SourceObject;

	/** Gameplay tags for this item entry. */
	UPROPERTY()
	FGameplayTagContainer DynamicTags;

	/**
	 * List of instanced item component data that are associated with this item.
	 */
	UPROPERTY()
	FItemComponentDataList ItemData;

	/** Current stack count of this item. */
	UPROPERTY()
	int32 StackCount;

	/** The last stack count that was locally observed. */
	UPROPERTY(NotReplicated)
	int32 LastObservedStackCount;

	/** Pending removal due to scope lock */
	UPROPERTY(NotReplicated)
	uint8 PendingRemove:1;

public:
	/** Returns this item entry as a debug string. */
	FString GetDebugString() const;

	/** Resets this item entry to an invalid state. */
	void Reset();

	//~ Begin FFastArraySerializerItem Interface
	void PreReplicatedRemove(const FInventoryItemContainer& InArraySerializer);
	void PostReplicatedAdd(const FInventoryItemContainer& InArraySerializer);
	//~ End FFastArraySerializerItem Interface

public:
	// ----------------------------------------------------------------------------------------------------------------
	// Operators
	// ----------------------------------------------------------------------------------------------------------------
	
	bool operator==(const FInventoryItemEntry& Other) const
	{
		return ItemHandle.Get() == Other.ItemHandle.Get();
	}

	bool operator==(const UInventoryItemInstance* OtherInstance) const
	{
		return Instance == OtherInstance;
	}

	bool operator==(const FInventoryItemHandle& OtherHandle) const
	{
		return ItemHandle.Get() == OtherHandle.Get();
	}

	bool operator>(const FInventoryItemEntry& Other) const
	{
		return StackCount > Other.StackCount;
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

/** Fast array serializer for inventory item entries. */
USTRUCT(BlueprintType)
struct FInventoryItemContainer : public FFastArraySerializer
{
	GENERATED_BODY()
	friend struct FInventoryItemEntry;
	friend class AInventory;
	friend class UInventoryItemInstance;

	FInventoryItemContainer();
	FInventoryItemContainer(AInventory* InOwningInventory);

public:
	//~ Begin FFastArraySerializer Interface
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryItemEntry, FInventoryItemContainer>(Items, DeltaParms, *this);
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

protected:
	/** List of item entries in this inventory. */
	UPROPERTY()
	TArray<FInventoryItemEntry> Items;

	/** Inventory class that owns this list. */
	UPROPERTY(NotReplicated)
	TObjectPtr<AInventory> OwningInventory;

public:
	/** TArray accessors for the items. */
	ITEMIZATION_FastArraySerializer_TArray_ACCESSORS(Items);
};

template<>
struct TStructOpsTypeTraits<FInventoryItemContainer> : TStructOpsTypeTraitsBase2<FInventoryItemContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithNetSharedSerialization = true,
	};
};