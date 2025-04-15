// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "InventoryChangeMessage.generated.h"

class AActor;
class AController;
class UInventoryItemInstance;
class UInventoryManager;
class AInventory;

/**
 * A generic message payload struct that can be used to send inventory change events.
 */
USTRUCT(BlueprintType)
struct FInventoryChangeMessage
{
	GENERATED_BODY()

	FInventoryChangeMessage() = default;
	FInventoryChangeMessage(TWeakObjectPtr<UInventoryItemInstance> ItemThatChanged, const int32 NewCount, const int32 OldCount)
		: ItemInstance(ItemThatChanged)
		, NewStackCount(NewCount)
		, Delta(NewCount - OldCount)
	{
	}

public:
	/** The actor that the inventory change occurred on. */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	TWeakObjectPtr<AActor> Owner = nullptr;

	/** Optional controller assigned to the inventory owner. */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	TWeakObjectPtr<AController> Controller = nullptr;

	/** The inventory class that the change occurred on. */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	TWeakObjectPtr<AInventory> Inventory = nullptr;

	/** The item instance that was changed (can be null in some edge cases). */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	TWeakObjectPtr<UInventoryItemInstance> ItemInstance = nullptr;

	/** The new stack count of the item. */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	int32 NewStackCount = 0;

	/** The delta stack count of the item. */
	UPROPERTY(BlueprintReadWrite, Category=Message)
	int32 Delta = 0;
};
