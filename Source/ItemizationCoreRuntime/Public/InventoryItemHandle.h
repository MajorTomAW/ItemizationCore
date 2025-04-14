// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "InventoryItemHandle.generated.h"

/**
 * Globally unique handle that points to a specific item entry in the inventory.
 *
 * With the usage of this handle, we can refer to a specific item entry in the inventory in a very efficient way,
 * without having to store the item entry itself.
 * An item entry gets referenced by a unique id/hash and a slot id, while the slot id might not always be needed/valid.
 * In cases where we're looking up the item entry via this handle, but the slot id is not valid, we will iterate through the inventory.
 * However, it is always preferred to use the slot id if we know it, as this will be much faster.
 */
USTRUCT(BlueprintType)
struct FInventoryItemHandle
{
	GENERATED_BODY()
	FInventoryItemHandle();

public:
	enum
	{
		HANDLE_MASK = 0x0000FFFF,		// Masks the lower 16 bits of the handle
		UID_SHIFT = 0x10,				// Shift the unique id by 16 bits
		INVALID_HANDLE = INDEX_NONE		// Invalid handle value
	};
	
	/** Clears the handle. */
	FORCEINLINE void ClearHandle()
	{
		Value = INVALID_HANDLE;
	}
	
	/** Checks if the handle is valid. */
	FORCEINLINE bool IsValid() const
	{
		return Value != INVALID_HANDLE;
	}

	/** Returns the handle as a slot id. */
	FORCEINLINE uint16 GetSlotId() const
	{
		return Value & HANDLE_MASK;
	}

	/** Returns the handle as a hash. */
	FORCEINLINE uint16 GetUID() const
	{
		return (uint16)(Value >> UID_SHIFT) & HANDLE_MASK;
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
	FORCEINLINE bool operator==(const FInventoryItemHandle& Other) const
	{
		return Value == Other.Value;
	}
	FORCEINLINE bool operator!=(const FInventoryItemHandle& Other) const
	{
		return Value != Other.Value;
	}

	/** Archive operator for serialization. */
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FInventoryItemHandle& SlotHandle)
	{
		static_assert(sizeof(FInventoryItemHandle) == 4, "If properties of FInventorySlotHandle change, consider updating this operator implementation.");
		Ar << SlotHandle.Value;
		return Ar;
	}

	/** Returns a hash value for this handle. */
	FORCEINLINE friend uint32 GetTypeHash(const FInventoryItemHandle& SlotHandle)
	{
		return ::GetTypeHash(SlotHandle.Value);
	}
	FORCEINLINE uint32 GetHash() const
	{
		return GetTypeHash(*this);
	}

	FORCEINLINE explicit operator bool() const
	{
		return IsValid();
	}

private:
	// The actual handle that points to the item entry.
	// First 2 bytes are used for the item slot id
	// Last 2 bytes are used for the unique id of the item entry
	uint32 Value;
};