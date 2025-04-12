// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "InventorySlotHandle.generated.h"

/**
 * Globally unique handle that points to a specific item entry in the inventory.
 *
 * With the usage of this handle, we can refer to a specific item entry in the inventory in a very efficient way,
 * without having to store the item entry itself.
 * This is useful in networking scenarios where we want to refer to an item entry in the inventory without having to replicate the entire item entry itself.
 */
USTRUCT(BlueprintType)
struct FInventorySlotHandle
{
	GENERATED_BODY()
	FInventorySlotHandle();

public:
	enum { HANDLE_MASK = 0x0000FFFF };
	
	/** Clears the handle. */
	FORCEINLINE void ClearHandle()
	{
		Value = HANDLE_MASK;
	}
	
	/** Checks if the handle is valid. */
	FORCEINLINE bool IsValid() const
	{
		return Value != HANDLE_MASK;
	}

	/** Returns the handle as a slot id. */
	FORCEINLINE uint16 GetSlotId() const
	{
		return Value & HANDLE_MASK;
	}

	/** Returns the handle as a hash. */
	FORCEINLINE uint16 GetHash() const
	{
		return (uint16)(Value >> 0x10) & HANDLE_MASK;
	}

	/** Converts the handle to a string. */
	FORCEINLINE FString ToString() const
	{
		return IsValid() ? FString::FromInt(Value) : FString("NULL");
	}

	/** Sets the slot id to the given value. */
	FORCEINLINE void SetSlotId(uint16 SlotId)
	{
		Value = (Value & ~HANDLE_MASK) | (SlotId & HANDLE_MASK);
	}

public:
	/** Compares this handle with another handle. */
	FORCEINLINE bool operator==(const FInventorySlotHandle& Other) const
	{
		return Value == Other.Value;
	}
	FORCEINLINE bool operator!=(const FInventorySlotHandle& Other) const
	{
		return Value != Other.Value;
	}

	/** Archive operator for serialization. */
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FInventorySlotHandle& SlotHandle)
	{
		static_assert(sizeof(FInventorySlotHandle) == 4, "If properties of FInventorySlotHandle change, consider updating this operator implementation.");
		Ar << SlotHandle.Value;
		return Ar;
	}

	/** Returns a hash value for this handle. */
	FORCEINLINE friend uint32 GetTypeHash(const FInventorySlotHandle& SlotHandle)
	{
		return ::GetTypeHash(SlotHandle.Value);
	}
	FORCEINLINE uint32 GetHash() const
	{
		return GetTypeHash(*this);
	}

private:
	// The actual handle that points to the item entry.
	// First 2 bytes are used for the item slot id
	// Last 2 bytes are used for hash
	uint32 Value;
};