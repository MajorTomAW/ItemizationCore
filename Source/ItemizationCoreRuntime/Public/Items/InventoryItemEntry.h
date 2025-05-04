// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryItemHandle.h"
#include "ItemizationCoreHelpers.h"
#include "ItemizationGameplayTags.h"
#include "Data/ItemComponentDataList.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventoryItemEntry.generated.h"

class AInventoryBase;
class UItemDefinitionBase;
class UInventoryItemInstance;
class UInventoryComponent;

/** Fast array serializer item for a single item entry in an inventory. */
USTRUCT(BlueprintType)
struct FInventoryItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	friend struct FInventoryItemContainer;
	friend class UInventoryComponent;

public:
	FInventoryItemEntry();
	FInventoryItemEntry(UItemDefinitionBase* InItemDefinition, int32 InStackSize, UObject* InSourceObj);

	/** Returns this item entry as a debug string. */
	FString GetDebugString() const;

	/** Prints out all stats associated with this item entry. */
	void DebugPrintStats() const;

	/** Resets this item entry to an invalid state. */
	void Reset();

	/** Returns the item instance associated with this entry. */
	UInventoryItemInstance* GetItemInstance() const;
	void SetReplicatedItemInstance(UInventoryItemInstance* InInstance);
	void SetNonReplicatedItemInstance(UInventoryItemInstance* InInstance);

	/** Returns a stat integer associated with the given tag. */
	int32 GetStatValue(const FGameplayTag& Tag) const;

	/** Sets the stat integer associated with the given tag. */
	void SetStatValue(const FGameplayTag& Tag, int32 Value);

	/** Returns all stat tags associated with this item. */
	const TMap<FGameplayTag, int32>& GetAllStats() const { return TagCountMap; }

	//~ Begin FFastArraySerializerItem Interface
	void PreReplicatedRemove(const FInventoryItemContainer& InArraySerializer);
	void PostReplicatedAdd(const FInventoryItemContainer& InArraySerializer);
	void PostReplicatedChange(const FInventoryItemContainer& InArraySerializer);
	//~ End FFastArraySerializerItem Interface
	
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

	UPROPERTY()
	FItemComponentDataList ItemData;

	/** The item definition that this entry represents. */
	UPROPERTY()
	TObjectPtr<UItemDefinitionBase> ItemDefinition;

	UPROPERTY()
	uint32 SlotNumber;

	/** */
	UPROPERTY()
	TWeakObjectPtr<UObject> SourceObject;

	/** The last stack count that was locally observed. */
	UPROPERTY(NotReplicated)
	int32 LastObservedStackCount;

	/** Pending removal due to scope lock */
	UPROPERTY(NotReplicated)
	uint8 bPendingRemove:1;

protected:
	/** Replicated item instance */
	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> ReplicatedInstance;

	/** Non-replicated item instance */
	UPROPERTY(NotReplicated)
	TObjectPtr<UInventoryItemInstance> NonReplicatedInstance;
	
	/** List of stat tags mapped to stat integer value. */
	UPROPERTY()
	TMap<FGameplayTag, int32> TagCountMap;

public:
	bool operator==(const FInventoryItemEntry& Other) const
	{
		return ItemHandle.Get() == Other.ItemHandle.Get();
	}

	bool operator==(const UInventoryItemInstance* OtherInstance) const
	{
		return GetItemInstance() == OtherInstance;
	}

	bool operator==(const FInventoryItemHandle& OtherHandle) const
	{
		return ItemHandle.Get() == OtherHandle.Get();
	}

	bool operator==(const UItemDefinitionBase* OtherDefinition) const
	{
		return ItemDefinition == OtherDefinition;
	}

	bool operator>(const FInventoryItemEntry& Other) const
	{
		return GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize)
		> Other.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
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
struct FInventoryItemContainer : public FFastArraySerializer
{
	GENERATED_BODY()
	friend class UInventoryComponent;
	friend class AInventoryBase;
	
public:
	FInventoryItemContainer();
	FInventoryItemContainer(AInventoryBase* InOwningInventory);
PRAGMA_DISABLE_DEPRECATION_WARNINGS
PRAGMA_ENABLE_DEPRECATION_WARNINGS

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
	
	/** TArray accessors for this container. */
	ITEMIZATION_FastArraySerializer_TArray_ACCESSORS(FInventoryItemContainer, FInventoryItemEntry, Items);
	
	/** List of item entries in this inventory. */
	UPROPERTY()
	TArray<FInventoryItemEntry> Items;

	/** The Inventory class that owns this list. */
	UPROPERTY(NotReplicated)
	TObjectPtr<AInventoryBase> OwningInventory;
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