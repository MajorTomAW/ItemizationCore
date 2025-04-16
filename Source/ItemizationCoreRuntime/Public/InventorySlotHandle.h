// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "InventorySlotHandle.generated.h"

/**
 * Globally unique handle that points to a specific item slot (+ entry) in the inventory.
 *
 * With the usage of this handle, we can refer to a specific item slot in the inventory in a very efficient way.
 * An item slot gets referenced by a slot id and a unique item id, while the item id might not always be needed/valid.
 * In cases where we're looking up the item entry via this handle, but the slot id is not valid, we will iterate through the inventory.
 * However, it is always preferred to use the slot id if we know it, as this will be much faster.
 */
USTRUCT(BlueprintType)
struct alignas(8) FInventorySlotHandle
{
	GENERATED_BODY()
	
	FInventorySlotHandle() = default;
	FInventorySlotHandle(const uint32 InSlotId, const uint32 InItemUID)
		: Value((static_cast<uint64>(InItemUID) << UID_SHIFT) | (InSlotId & UID_MASK))
	{
	}

public:
	enum
	{
		UID_MASK = 0x00000000FFFFFFFF,			// Masks the lower 32 bits of the handle
		UID_SHIFT = 0x20,							// Shift the unique id by 32 bits
		INVALID_HANDLE = INDEX_NONE						// Invalid handle value
	};
	

	/** Returns the handle as a slot id. */
	FORCEINLINE uint32 GetSlotId() const
	{
		return static_cast<uint32>(Value & UID_MASK);
	}

	/** Returns the handle as a hash. */
	FORCEINLINE uint32 GetUID() const
	{
		return static_cast<uint32>((Value >> UID_SHIFT) & UID_MASK);
	}

	/** Returns this handles raw value. */
	uint64 Get() const
	{
		return Value;
	}

	/** Converts the handle to a string. */
	FString ToString() const
	{
		return IsValid() ? FString::Printf(TEXT("0x%016llX|(%llu)"), Value, Value) : TEXT("NullHandle");
	}

	/** Sets the slot id to the given value. */
	void SetSlotId(uint32 SlotId)
	{
		Value = (Value & ~UID_MASK) | (SlotId & UID_MASK);
	}

	/** Sets the item uid to the given value. */
	void SetUID(uint32 ItemUID)
	{
		Value = (Value & UID_MASK) | (static_cast<uint64>(ItemUID) << UID_SHIFT);
	}
	
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
		return IsSlotValid() && IsUIDValid();
	}

	/** Checks if the slot id is a valid id. */
	FORCEINLINE bool IsSlotValid() const
	{
		return GetSlotId() != INVALID_HANDLE;
	}

	/** Checks ot see if the UID was set. */
	FORCEINLINE bool IsUIDValid() const
	{
		return GetUID() != 0x0;
	}

public:
	/** Compares this handle with another handle. */
	bool operator==(const FInventorySlotHandle& Other) const
	{
		return Get() == Other.Get();
	}
	bool operator!=(const FInventorySlotHandle& Other) const
	{
		return !operator==(Other);
	}

	/** For sorting purposes. */
	bool operator<(const FInventorySlotHandle& Other) const { return GetSlotId() < Other.GetSlotId(); }

	/** Archive operator for serialization. */
	friend FArchive& operator<<(FArchive& Ar, FInventorySlotHandle& SlotHandle)
	{
		Ar << SlotHandle.Value;
		return Ar;
	}

	/** Returns a hash value for this handle. */
	friend uint32 GetTypeHash(const FInventorySlotHandle& SlotHandle)
	{
		return ::GetTypeHash(SlotHandle.Value);
	}
	uint32 GetHash() const
	{
		return GetTypeHash(*this);
	}

private:
	// The actual handle that points to the item slot.
	// – First 2 bytes: Item slot id (Range: 0 to 4,294,967,295).
	// – Last 2 bytes: UID of the item entry (Range: 0 to 4,294,967,295).
	UPROPERTY()
	uint64 Value = INVALID_HANDLE;
};

static_assert(sizeof(FInventorySlotHandle) == sizeof(uint64), "Expected FInventorySlotHandle to be 8 bytes.");
static_assert(alignof(FInventorySlotHandle) == sizeof(uint64), "Expected FInventorySlotHandle to be 8 bytes.");