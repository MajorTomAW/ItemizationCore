// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemEntryHandle.h"
#include "ItemizationCoreTypes.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventoryItemEntry.generated.h"

class UItemDefinition;
class UInventoryItemInstance;
class UInventoryManager;

/**
 * Fast array serializer for inventory item entries.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventoryItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
public:
	friend struct FInventoryItemContainer;
	friend class UInventoryItemInstance;
	friend class UInventoryManager;

	FInventoryItemEntry();
	FInventoryItemEntry(UItemDefinition* ItemDefinition, int32 InStackCount, UObject* InSourceObject = nullptr);

	/** Handle for outside sources to refer to this entry by */
	UPROPERTY()
	FInventoryItemEntryHandle Handle;

	/** The definition of the item */
	UPROPERTY()
	TObjectPtr<UItemDefinition> Definition;

	/** Instance of the item entry */
	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> Instance;

	/** Stack count of the item */
	UPROPERTY()
	int32 StackCount;

	/** The last stack count that was observed */
	UPROPERTY(NotReplicated)
	int32 LastObservedStackCount;

	/** Object this item was given from, can be an actor or static object. Useful to find out previous ownership */
	UPROPERTY()
	TWeakObjectPtr<UObject> SourceObject;

	/** Authority-only list of granted handles. */
	UPROPERTY(NotReplicated)
	FItemizationGrantedHandles GrantedHandles;

	/** Pending removal due to scope lock */
	UPROPERTY(NotReplicated)
	uint8 bPendingRemove : 1;

	/** Returns true if the item's stack count is full. */
	bool IsStackFull() const;

	/** Returns the current stack count of the item. */
	FORCEINLINE int32 GetStackCount() const { return StackCount; }

	//~ Begin FFastArraySerializerItem Interface
	void PreReplicatedRemove(const FInventoryItemContainer& InArraySerializer);
	void PostReplicatedAdd(const FInventoryItemContainer& InArraySerializer);
	//~ End FFastArraySerializerItem Interface
};

/** Fast array serializer wrapper for the above struct. */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventoryItemContainer : public FFastArraySerializer
{
public:
	GENERATED_USTRUCT_BODY()

	FInventoryItemContainer()
		: Owner(nullptr)
	{
	}

	template<typename Type, typename SerializerType>
	bool ShouldWriteFastArrayItem(const Type& Item, const bool bIsWritingOnClient)
	{
		if (bIsWritingOnClient)
		{
			return Item.ReplicationID != INDEX_NONE;
		}

		return true;
	}

	//~ Begin FFastArraySerializer Interface
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~ End FFastArraySerializer Interface)

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryItemEntry, FInventoryItemContainer>(Items, DeltaParms, *this);
	}
	

	/** Initializes Owner variable. */
	void RegisterWithOwner(UInventoryManager* InOwner);

	/** Gets a list of all items in the inventory of the specified type. */
	void GetItemsOfType(TArray<FInventoryItemEntry*>* OutItems, const UItemDefinition* ItemDefinitionType, bool bIncompleteStacksOnly = false);

#if WITH_GAMEPLAY_MESSAGE_ROUTER
	/** Constructs and broadcasts a change message for the given item. */
	void BroadcastInventoryChangeMessage(UInventoryItemInstance* ItemThatChanged, int32 OldStackCount, int32 NewStackCount) const;
#endif

	/** List of items in the inventory */
	UPROPERTY()
	TArray<FInventoryItemEntry> Items;

	/** Component that owns this list */
	UPROPERTY(NotReplicated)
	TObjectPtr<UInventoryManager> Owner;
};

template<>
struct TStructOpsTypeTraits<FInventoryItemContainer> : public TStructOpsTypeTraitsBase2<FInventoryItemContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

/** Used to stop us from removing abilities from an inventory maanger while we're iterating through the items */
struct ITEMIZATIONCORERUNTIME_API FScopedInventoryListLock
{
	FScopedInventoryListLock(UInventoryManager& InManager);
	~FScopedInventoryListLock();

private:
	UInventoryManager& InventoryManager;
};

#define INVENTORY_LIST_SCOPE_LOCK() FScopedInventoryListLock ActiveScopeLock(*this);
