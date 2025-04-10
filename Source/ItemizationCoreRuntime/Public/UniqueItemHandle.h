// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "UniqueItemHandle.generated.h"

/**
 * Globally unique handle that points to a specific item entry in the inventory.
 *
 * With the usage of this handle, we can refer to a specific item entry in the inventory in a very efficient way,
 * without having to store the item entry itself.
 * This is useful in networking scenarios where we want to refer to an item entry in the inventory without having to replicate the entire item entry itself.
 */
USTRUCT(BlueprintType)
struct FUniqueItemHandle
{
	GENERATED_BODY()
	FUniqueItemHandle();

public:
	/** Generates a new valid handle. */
	void GenerateNewHandle();

	/** Clears the handle. */
	FORCEINLINE void ClearHandle()
	{
		Handle = INDEX_NONE;
	}
	
	/** Checks if the handle is valid. */
	FORCEINLINE bool IsValid() const
	{
		return Handle != INDEX_NONE;
	}

	/** Returns the raw handle value. */
	FORCEINLINE int32 Get() const
	{
		return Handle;
	}

	/** Converts the handle to a string. */
	FORCEINLINE FString ToString() const
	{
		return IsValid() ? FString::FromInt(Handle) : FString("NULL");
	}

	/** An invalid item handle. */
	static const FUniqueItemHandle NullHandle;

public:
	/** Compares this handle with another handle. */
	FORCEINLINE bool operator==(const FUniqueItemHandle& Other) const
	{
		return Handle == Other.Handle;
	}
	FORCEINLINE bool operator!=(const FUniqueItemHandle& Other) const
	{
		return Handle != Other.Handle;
	}

	/** Archive operator for serialization. */
	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FUniqueItemHandle& ItemHandle)
	{
		static_assert(sizeof(FUniqueItemHandle) == 4, "If properties of FUniqueItemHandle change, consider updating this operator implementation.");
		Ar << ItemHandle.Handle;
		return Ar;
	}

	/** Returns a hash value for this handle. */
	FORCEINLINE friend uint32 GetTypeHash(const FUniqueItemHandle& ItemHandle)
	{
		return ::GetTypeHash(ItemHandle.Handle);
	}

private:
	/** The actual handle that points to the item entry. */
	UPROPERTY()
	int32 Handle;
};