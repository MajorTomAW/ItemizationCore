// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventorySlotHandle.h"
#include "ItemizationCoreHelpers.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventorySlotEntry.generated.h"

/**
 * Fast array serializer for inventory slots.
 * This struct represents a single slot in an inventory system.
 *
 * A slot can be associated with an item entry which lives in the inventory.
 * It gets referenced by its item handle to save performance and bandwidth.
 *
 * When this slot is empty, the item handle will be invalid.
 */
USTRUCT(BlueprintType)
struct FInventorySlotEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	friend struct FInventorySlotContainer;
	friend class ASlottableInventory;

public:
	FInventorySlotEntry();
	explicit FInventorySlotEntry(uint16 InSlotId);
	explicit FInventorySlotEntry(const FInventorySlotHandle& InItemHandle);

	/** Returns the item handle associated with this slot. */
	FORCEINLINE FInventorySlotHandle GetHandle() const
	{
		return SlotHandle;
	}
	FInventorySlotHandle& GetHandle_Ref()
	{
		return SlotHandle;
	}

	/** Returns the item handle as an uint16 slot id. */
	FORCEINLINE uint16 GetSlotId() const
	{
		return SlotHandle.GetSlotId();
	}

	/** Returns whether this slot is enabled. */
	FORCEINLINE bool IsEnabled() const
	{
		return bEnabled;
	}
	
	/** Sets whether this slot is enabled. */
	void SetEnabled(bool bInEnabled)
	{
		bEnabled = bInEnabled;
	}

	/** Resets this slot to an empty state. */
	void Reset();

	/**
	 * Checks if this slot entry is valid.
	 * Will return true if the slot id is valid, but won't check if the item handle is valid.
	 */
	bool IsValid() const;

	// ----------------------------------------------------------------------------------------------------------------
	// Operators
	// ----------------------------------------------------------------------------------------------------------------

	FORCEINLINE bool operator==(const FInventorySlotEntry& Other) const
	{
		return GetHandle() == Other.GetHandle();
	}
	FORCEINLINE bool operator==(const FInventorySlotHandle& OtherHandle) const
	{
		return GetHandle() == OtherHandle;
	}

	/** Returns this slot as a debug string. */
	FString ToString() const
	{
		return FString::Printf(TEXT("Slot[%d][%s]"), GetSlotId(), *GetHandle().ToString());
	}

	//~ Begin FFastArraySerializerItem Interface
	void PreReplicatedRemove(const FInventorySlotContainer& InArraySerializer);
	void PostReplicatedAdd(const FInventorySlotContainer& InArraySerializer);
	void PostReplicatedChange(const FInventorySlotContainer& InArraySerializer);
	//~ End FFastArraySerializerItem Interface

protected:
	/** The item handle that both defines this slot id and the associated item entry. */
	UPROPERTY()
	FInventorySlotHandle SlotHandle;

	/** Whether this slot is enabled, meaning it can be used to modify the item which is in it. */
	UPROPERTY(BlueprintReadOnly, Category=Slot)
	uint8 bEnabled : 1;

	/** Optional gameplay tag associated with this slot. Can be used for trait checks or filtering. */
	UPROPERTY(BlueprintReadOnly, Category=Slot)
	FGameplayTag SlotTag;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotEntry> : TStructOpsTypeTraitsBase2<FInventorySlotEntry>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithNetSharedSerialization = true,
	};
};

/** Fast array serializer for inventory slots. */
USTRUCT(BlueprintType)
struct FInventorySlotContainer : public FFastArraySerializer
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

public:
	/** List of slots in this inventory. */
	UPROPERTY()
	TArray<FInventorySlotEntry> Slots;

public:
	/** TArray accessors for the slots. */
	ITEMIZATION_FastArraySerializer_TArray_ACCESSORS(Slots);
};

template<>
struct TStructOpsTypeTraits<FInventorySlotContainer> : public TStructOpsTypeTraitsBase2<FInventorySlotContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithNetSharedSerialization = true,
	};
};