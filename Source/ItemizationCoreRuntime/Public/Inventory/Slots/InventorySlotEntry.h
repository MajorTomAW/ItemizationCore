// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemHandle.h"
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

	/** Returns the item handle associated with this slot. */
	FORCEINLINE FInventoryItemHandle GetHandle() const
	{
		return Handle;
	}
	FORCEINLINE FInventoryItemHandle& GetHandle_Ref()
	{
		return Handle;
	}

	/** Returns the item handle as an uint16 slot id. */
	FORCEINLINE uint16 GetSlotId() const
	{
		return Handle.GetSlotId();
	}

	/** Returns whether this slot is enabled. */
	FORCEINLINE bool IsEnabled() const
	{
		return bEnabled;
	}
	/** Sets whether this slot is enabled. */
	FORCEINLINE void SetEnabled(bool bInEnabled)
	{
		bEnabled = bInEnabled;
	}

	// ----------------------------------------------------------------------------------------------------------------
	// Operators
	// ----------------------------------------------------------------------------------------------------------------

	FORCEINLINE bool operator==(const FInventorySlotEntry& Other) const
	{
		return GetHandle() == Other.GetHandle();
	}
	FORCEINLINE bool operator==(const FInventoryItemHandle& OtherHandle) const
	{
		return GetHandle() == OtherHandle;
	}

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
	FInventoryItemHandle Handle;

	/** Whether this slot is enabled, meaning it can be used to modify the item which is in it. */
	UPROPERTY()
	uint8 bEnabled : 1;
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
	// ----------------------------------------------------------------------------------------------------------------
	// Operators
	// ----------------------------------------------------------------------------------------------------------------

	/** Returns the number of slots in this container. */
	FORCEINLINE int32 Num() const
	{
		return Slots.Num();
	}

	/** Creates a const iterator for this container. */
	FORCEINLINE TArray<FInventorySlotEntry>::TConstIterator CreateConstIterator() const
	{
		return Slots.CreateConstIterator();
	}

	/** Creates an iterator for this container. */
	FORCEINLINE TArray<FInventorySlotEntry>::TIterator CreateIterator()
	{
		return Slots.CreateIterator();
	}

private:
	FORCEINLINE friend TArray<FInventorySlotEntry>::TConstIterator begin(const FInventorySlotContainer& Array)
	{
		return Array.CreateConstIterator();
	}

	FORCEINLINE friend TArray<FInventorySlotEntry>::TConstIterator end(const FInventorySlotContainer& Array)
	{
		return TArray<FInventorySlotEntry>::TConstIterator(Array.Slots, Array.Num());
	}

	FORCEINLINE friend TArray<FInventorySlotEntry>::TIterator begin(FInventorySlotContainer& Array)
	{
		return Array.CreateIterator();
	}

	FORCEINLINE friend TArray<FInventorySlotEntry>::TIterator end(FInventorySlotContainer& Array)
	{
		return TArray<FInventorySlotEntry>::TIterator(Array.Slots, Array.Num());
	}
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