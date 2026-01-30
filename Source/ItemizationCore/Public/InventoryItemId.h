// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryItemId.generated.h"

/** Globally unique handle that points to an exact FInventoryItemEntry in an inventory. */
USTRUCT(BlueprintType)
struct alignas(4) FInventoryItemId
{
	GENERATED_BODY()

	FInventoryItemId() : UID(INVALID_HANDLE) {}
	explicit FInventoryItemId(uint32 InUID) : UID(InUID) {}

public:
	enum
	{
		INVALID_HANDLE = 0, // Invalid handle value
	};

	/** Generates a new uid and sets it to this handle. */
	void GenerateNewId();

	/** Returns this handles raw value. */
	[[nodiscard]] uint32 Get() const
	{
		return UID;
	}

	/** Converts this handle to a string. */
	[[nodiscard]] FString ToString() const
	{
		return IsValid() ? FString::Printf(TEXT("0x%08X|(%lu)"), UID, UID) : TEXT("NullHandle");
	}

	/** Resets this handle to an invalid state. */
	void Reset()
	{
		UID = INVALID_HANDLE;
	}

	/** Checks if this handle is valid. */
	inline bool IsValid() const
	{
		return UID != INVALID_HANDLE;
	}

	static FInventoryItemId InvalidId;

public:
	/** Compares this handle with another handle. */
	bool operator==(const FInventoryItemId& Other) const
	{
		return Get() == Other.Get();
	}
	bool operator!=(const FInventoryItemId& Other) const
	{
		return !operator==(Other);
	}

	/** For sorting purposes. */
	bool operator<(const FInventoryItemId& Other) const { return Get() < Other.Get(); }

	/** Archive operator for serialization. */
	friend FArchive& operator<<(FArchive& Ar, FInventoryItemId& ItemHandle)
	{
		Ar << ItemHandle.UID;
		return Ar;
	}

	/** Returns a hash value for this handle. */
	friend uint32 GetTypeHash(const FInventoryItemId& ItemHandle)
	{
		return ::GetTypeHash(ItemHandle.UID);
	}
	uint32 GetHash() const
	{
		return GetTypeHash(*this);
	}

	explicit operator bool() const
	{
		return IsValid();
	}

	explicit operator uint32() const
	{
		return Get();
	}

private:
	/** The actual handle that points to the item entry. */
	UPROPERTY()
	uint32 UID = INVALID_HANDLE;
};

static_assert(sizeof(FInventoryItemId) == sizeof(uint32), "Expected FInventoryItemId to be 4 bytes.");
static_assert(alignof(FInventoryItemId) == sizeof(uint32), "Expected FInventoryItemId to be aligned to 4 bytes.");
