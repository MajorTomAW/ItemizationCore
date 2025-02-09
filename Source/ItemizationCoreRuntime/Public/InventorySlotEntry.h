// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemEntryHandle.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventorySlotEntry.generated.h"

class UItemDefinition;
class UInventoryItemInstance;
class UInventorySlotManager;
class UInventoryManager;
struct FInventoryItemEntry;
struct FInventorySlotContainer;

/**
 * Fast array serializer for inventory slot entries.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventorySlotEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	friend struct FInventoryItemEntry;
	friend class UInventoryItemInstance;
	friend class UInventorySlotManager;
	friend class UInventoryManager;

	FInventorySlotEntry();
	explicit FInventorySlotEntry(const FInventoryItemEntryHandle& InHandle, int32 InSlotIndex = INDEX_NONE);

public:
	/** The item entry that this slot entry represents. */
	UPROPERTY()
	FInventoryItemEntryHandle Handle;

	/** The slot index that this entry is in. */
	UPROPERTY()
	int32 SlotIndex;

	/** Returns true if this slot entry is valid. Aka, it has an item assigned to it. */
	FORCEINLINE bool IsValid() const { return Handle.IsValid(); }

	//~ Begin FFastArraySerializerItem Interface
	void PreReplicatedRemove(const FInventorySlotContainer& InArraySerializer);
	void PostReplicatedAdd(const FInventorySlotContainer& InArraySerializer);
	void PostReplicatedChange(const FInventorySlotContainer& InArraySerializer);
	//~ End FFastArraySerializerItem Interface
};

/** Fast array serializer for inventory slot entries. */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventorySlotContainer : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FInventorySlotContainer();

	//~ Begin FFastArraySerializer Interface
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventorySlotEntry, FInventorySlotContainer>(Slots, DeltaParms, *this);
	}
	//~ End FFastArraySerializer Interface

	/** List of inventory slot entries. */
	UPROPERTY()
	TArray<FInventorySlotEntry> Slots;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotContainer> : public TStructOpsTypeTraitsBase2<FInventorySlotContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};