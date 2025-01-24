// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InventoryItemEntryHandle.generated.h"

/**
 * Globally unique handle that points to a specific item spec in the inventory.
 *
 * With the usage of this handle, we can refer to a specific item spec in the inventory in a very efficient way,
 * without having to store the item spec itself.
 * This is useful in networking scenarios where we want to refer to an item spec in the inventory without having to replicate the item spec itself.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventoryItemEntryHandle
{
	GENERATED_USTRUCT_BODY()

public:
	FInventoryItemEntryHandle();

	/** True if this handle is valid (GenerateNewHandle was called). */
	bool IsValid() const
	{
		return Handle != INDEX_NONE;
	}

	/** Sets this to a valid handle. */
	void GenerateNewHandle();

	bool operator==(const FInventoryItemEntryHandle& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator!=(const FInventoryItemEntryHandle& Other) const
	{
		return Handle != Other.Handle;
	}

	/** Operator to expose FInventoryItemEntryHandle serialization to custom serialization functions like NetSerialize overrides. */
	friend FArchive& operator<<(FArchive& Ar, FInventoryItemEntryHandle& ItemSpecHandle)
	{
		static_assert(sizeof(FInventoryItemEntryHandle) == 4, "If properties of FInventoryItemEntryHandle change, consider updating this operator implementation.");
		Ar << ItemSpecHandle.Handle;
		return Ar;
	}

	friend uint32 GetTypeHash(const FInventoryItemEntryHandle& SpecHandle)
	{
		return ::GetTypeHash(SpecHandle.Handle);
	}

	FString ToString() const
	{
		return IsValid() ? FString::FromInt(Handle) : TEXT("Invalid");	
	}

	/** Returns the handle as an integer. */
	int32 Get() const
	{
		return Handle;
	}

	/** An invalid handle. */
	static const FInventoryItemEntryHandle NullHandle;

private:
	/** Handle to the item spec. */
	UPROPERTY()
	int32 Handle;
};
