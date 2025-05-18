// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryChangeMessage.generated.h"

struct FInventoryItemEntry;
class UObject;
class AActor;
class AController;
class AInventoryBase;

/** Generic message payload that gets passed around when an inventory change occurs. */
USTRUCT(BlueprintType)
struct FInventoryChangeMessage
{
	GENERATED_BODY()

	FInventoryChangeMessage() = default;
	FInventoryChangeMessage(const FInventoryItemEntry* ItemThatChanged, const int32 NewCount, const int32 OldCount)
		: ItemEntry(ItemThatChanged)
		, NewStackCount(NewCount)
		, Delta(NewCount - OldCount)
	{
	}

	virtual ~FInventoryChangeMessage() = default;

public:
	/** The actor that the inventory change occurred on. */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	TWeakObjectPtr<AActor> Owner = nullptr;

	/** Optional controller assigned to the inventory owner. */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	TWeakObjectPtr<AController> Controller = nullptr;

	/** The inventory class that the change was instigated on. */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	TWeakObjectPtr<AInventoryBase> SourceInventory = nullptr;

	/** The inventory class that the change occurred on. */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	TWeakObjectPtr<AInventoryBase> TargetInventory = nullptr;

	/** The item entry that was changed (can be null in some edge cases). */
	const FInventoryItemEntry* ItemEntry = nullptr;

	/** The new stack count of the item. */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	int32 NewStackCount = 0;

	/** The delta stack count of the item. */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	int32 Delta = 0;
};