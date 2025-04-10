// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemComponentDataList.h"
#include "UniqueItemHandle.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventoryItemEntry.generated.h"

/** Fast array serializer for a single item entry in an inventory. */
USTRUCT(BlueprintType)
struct FInventoryItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	friend struct FInventoryItemContainer;
	friend class AInventoryBase;

	FInventoryItemEntry();

public:
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	FInventoryItemEntry(const FInventoryItemEntry&) = default;
	FInventoryItemEntry(FInventoryItemEntry&&) = default;
	FInventoryItemEntry& operator=(const FInventoryItemEntry&) = default;
	FInventoryItemEntry& operator=(FInventoryItemEntry&&) = default;
	~FInventoryItemEntry() = default;
PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
	/** The unique handle to this item for outside references. */
	UPROPERTY()
	FUniqueItemHandle UniqueHandle;
	
	/**
	 * Object that this item was given from.
	 * Can be an actor or a static object.
	 * Useful to find previous ownership or to find out where this item came from.
	 */
	UPROPERTY()
	TWeakObjectPtr<UObject> SourceObject;

	/**
	 * List of instanced item component data that are associated with this item.
	 */
	UPROPERTY()
	FItemComponentDataList ItemData;
};

/** Fast array serializer for inventory item entries. */
USTRUCT(BlueprintType)
struct FInventoryItemContainer : public FFastArraySerializer
{
	GENERATED_BODY()
	friend struct FInventoryItemEntry;
	friend class AInventoryBase;

	FInventoryItemContainer();

public:

protected:
	/** List of item entries in this inventory. */
	UPROPERTY()
	TArray<FInventoryItemEntry> Items;
};