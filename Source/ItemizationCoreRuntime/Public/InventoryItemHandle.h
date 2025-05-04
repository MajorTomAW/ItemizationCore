// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryItemHandle.generated.h"

/** Globally unique handle that points to an exact FInventoryItemEntry in an inventory. */	
USTRUCT(BlueprintType)
struct alignas(4) FInventoryItemHandle
{
	GENERATED_BODY()

	FInventoryItemHandle() = default;

public:
	enum
	{
		INVALID_HANDLE = 0x0,		// Invalid handle value
	};

	/** Generates a new uid and sets it to this handle. */
	void GenerateNewUID();

	/** Returns this handles raw value. */
	uint32 Get() const
	{
		return UID;
	}

	/** Converts this handle to a string. */
	FString ToString() const
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

public:
	/** Compares this handle with another handle. */
	bool operator==(const FInventoryItemHandle& Other) const
	{
		return Get() == Other.Get();
	}
	bool operator!=(const FInventoryItemHandle& Other) const
	{
		return !operator==(Other);
	}

	/** For sorting purposes. */
	bool operator<(const FInventoryItemHandle& Other) const { return Get() < Other.Get(); }

	/** Archive operator for serialization. */
	friend FArchive& operator<<(FArchive& Ar, FInventoryItemHandle& ItemHandle)
	{
		Ar << ItemHandle.UID;
		return Ar;
	}

	/** Returns a hash value for this handle. */
	friend uint32 GetTypeHash(const FInventoryItemHandle& ItemHandle)
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

static_assert(sizeof(FInventoryItemHandle) == sizeof(uint32), "Expected FInventoryItemHandle to be 4 bytes.");
static_assert(alignof(FInventoryItemHandle) == sizeof(uint32), "Expected FInventoryItemHandle to be aligned to 4 bytes.");