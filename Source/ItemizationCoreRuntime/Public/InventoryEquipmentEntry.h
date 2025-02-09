// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemEntry.h"
#include "ItemizationCoreTypes.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventoryEquipmentEntry.generated.h"

class UInventoryEquipmentInstance;
class UEquipmentManager;
class UItemDefinition;
class UInventoryManager;

/**
 * Fast array serializer for the inventory equipment entry.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventoryEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()
	friend class UEquipmentManager;
	friend struct FInventoryEquipmentContainer;

	FInventoryEquipmentEntry();
	explicit FInventoryEquipmentEntry(const FInventoryItemEntryHandle& InHandle);

public:
	/** The item entry that owns this equipment. */
	UPROPERTY()
	FInventoryItemEntryHandle Handle;

	/** The definition of the item */
	UPROPERTY()
	TObjectPtr<UItemDefinition> Definition;

	/** Instance of the equipment item */
	UPROPERTY()
	TObjectPtr<UInventoryEquipmentInstance> Instance;

	/** Object this item was equipped from. Should always be the UInventoryItemInstance. */
	UPROPERTY()
	TObjectPtr<UObject> SourceObject;

	/** Authority-only list of granted handles. */
	UPROPERTY(NotReplicated)
	FItemizationGrantedHandles GrantedHandles;

	//~ Begin FFastArraySerializerItem Interface
	void PreReplicatedRemove(const FInventoryEquipmentContainer& InArraySerializer);
	void PostReplicatedAdd(const FInventoryEquipmentContainer& InArraySerializer);
	void PostReplicatedChange(const FInventoryEquipmentContainer& InArraySerializer);
	//~ End FFastArraySerializerItem Interface
};

/** Fast array serializer wrapper for the above struct. */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventoryEquipmentContainer : public FFastArraySerializer
{
public:
	GENERATED_USTRUCT_BODY()

	FInventoryEquipmentContainer()
		: Owner(nullptr)
	{
	}

	//~ Begin FFastArraySerializer Interface
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~ End FFastArraySerializer Interface

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryEquipmentEntry, FInventoryEquipmentContainer>(Items, DeltaParms, *this);
	}

	/** Initializes Owner variable. */
	void RegisterWithOwner(UEquipmentManager* InOwner);
	
	/** List of equipment entries */
	UPROPERTY()
	TArray<FInventoryEquipmentEntry> Items;

	/** Component that owns this list */
	UPROPERTY(NotReplicated)
	TObjectPtr<UEquipmentManager> Owner;
};

template<>
struct TStructOpsTypeTraits<FInventoryEquipmentContainer> : public TStructOpsTypeTraitsBase2<FInventoryEquipmentContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};