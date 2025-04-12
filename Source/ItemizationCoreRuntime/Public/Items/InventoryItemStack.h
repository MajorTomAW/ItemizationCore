// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "InventoryItemStack.generated.h"

class UInventoryItemInstance;
class UPackage;
class UPackageMap;

/**
 * Struct that represents a single stack of items in the inventory.
 */
USTRUCT(BlueprintType)
struct FInventoryItemStack
{
	GENERATED_BODY()

	FInventoryItemStack();

public:
	/**
	 * The actual item instance that this entry is representing.
	 * Invalid by default, gets created by the AInventoryBase::OnGiveItem() function.
	 */
	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> Instance;

public:
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
	bool operator==(const FInventoryItemStack&) const;
};

template <>
struct TStructOpsTypeTraits<FInventoryItemStack> : public TStructOpsTypeTraitsBase2<FInventoryItemStack>
{
	enum
	{
		WithNetSerializer = true,
		WithIdenticalViaEquality = true,
	};
};