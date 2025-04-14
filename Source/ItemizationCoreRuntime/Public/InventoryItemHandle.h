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
struct alignas(8) FInventoryItemHandle
{
	GENERATED_BODY()
	
	FInventoryItemHandle() = default;
	FInventoryItemHandle(const uint32 InSlotId, const uint32 InItemUID)
		: Value((static_cast<uint64>(InItemUID) << UID_SHIFT) | (InSlotId & HANDLE_MASK))
	{
	}

public:
	enum
	{
		HANDLE_MASK = 0x00000000FFFFFFFF,			// Masks the lower 32 bits of the handle
		UID_SHIFT = 0x20,							// Shift the unique id by 32 bits
		INVALID_HANDLE = 0x0						// Invalid handle value
	};
	
	/** Clears the handle. */
	void Reset()
	{
		Value = INVALID_HANDLE;
	}
	
	/**
	 * Checks if the handle is valid.
	 * @Note This will always return true if either SlotId or ItemUID is valid.
	 * @Note It is safer to check IsSlotValid() or IsUIDValid() if you want to check for a specific value.
	 */
	FORCEINLINE bool IsValid() const
	{
		return Value != INVALID_HANDLE;
	}

	/** Checks if the slot id is a valid id. */
	FORCEINLINE bool IsSlotValid() const
	{
		return (Value & HANDLE_MASK) != INVALID_HANDLE;
	}

	/** Checks ot see if the UID was set. */
	FORCEINLINE bool IsUIDValid() const
	{
		return (Value >> UID_SHIFT) != INVALID_HANDLE;
	}

	/** Returns the handle as a slot id. */
	FORCEINLINE uint32 GetSlotId() const
	{
		return Value & HANDLE_MASK;
	}

	/** Returns the handle as a hash. */
	FORCEINLINE uint32 GetUID() const
	{
		return (Value >> UID_SHIFT) & HANDLE_MASK;
	}

	/** Returns this handles raw value. */
	uint64 Get() const
	{
		return Value;
	}

	/** Converts the handle to a string. */
	FString ToString() const
	{
		return IsValid() ? FString::FromInt(Value) : FString("NULL");
	}

	/** Sets the slot id to the given value. */
	void SetSlotId(uint32 SlotId)
	{
		Value = (Value & ~HANDLE_MASK) | (SlotId & HANDLE_MASK);
	}

public:
	/** Compares this handle with another handle. */
	bool operator==(const FInventoryItemHandle& Other) const
	{
		return Get() == Other.Get();
	}
	bool operator!=(const FInventoryItemHandle& Other) const
	{
		return Get() != Other.Get();
	}

	/** Archive operator for serialization. */
	friend FArchive& operator<<(FArchive& Ar, FInventoryItemHandle& SlotHandle)
	{
		Ar << SlotHandle.Value;
		return Ar;
	}

	/** Returns a hash value for this handle. */
	friend uint32 GetTypeHash(const FInventoryItemHandle& SlotHandle)
	{
		return ::GetTypeHash(SlotHandle.Value);
	}
	uint32 GetHash() const
	{
		return GetTypeHash(*this);
	}

	explicit operator bool() const
	{
		return IsValid();
	}

private:
	// The actual handle that points to the item entry.
	// – First 2 bytes: Item slot id (Range: 0 to 4,294,967,295).
	// – Last 2 bytes: UID of the item entry (Range: 0 to 4,294,967,295).
	uint64 Value = INVALID_HANDLE;
};

static_assert(sizeof(FInventoryItemHandle) == sizeof(uint64), "Expected FInventoryItemHandle to be 8 bytes.");
static_assert(alignof(FInventoryItemHandle) == sizeof(uint64), "Expected FInventoryItemHandle to be 8 bytes.");