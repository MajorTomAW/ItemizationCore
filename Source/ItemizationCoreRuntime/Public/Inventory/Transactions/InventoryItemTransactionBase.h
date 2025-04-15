// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "InventoryItemTransactionBase.generated.h"

struct FGameplayTagContainer;
class AInventory;
class AInventoryBase;
/**
 * Struct defining a base transactable block for inventory items.
 * This is used to define payloads for certain actions that can be performed on items or inventories.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventoryItemTransactionBase
{
	GENERATED_BODY()

	FInventoryItemTransactionBase() = default;
	FInventoryItemTransactionBase(UObject* InInstigator)
		: Instigator(InInstigator)
	{
	}
	virtual ~FInventoryItemTransactionBase() = default;

	/** Called to undo/revert the transaction. */
	virtual bool Undo() { return false; }

	/** Called to redo the transaction. */
	virtual bool Redo() { return false; }

public:
	/** Optional instigator of the transaction. */
	TWeakObjectPtr<UObject> Instigator;

	/** Stores the transaction index, which is used to identify the transaction. */
	int32 Index = INDEX_NONE;
};

class FScopedInventoryItemTransaction
{
public:
};

/** Inventory transaction struct for giving / removing items from an inventory. */
struct FInventoryTransaction_GiveItem : FInventoryItemTransactionBase
{
public:
	FInventoryTransaction_GiveItem() = default;

	/** The target inventory that we want to give the item to. */
	AInventory* TargetInventory = nullptr;

	/** The delta amount of items to give/remove. */
	int32 Delta = 0;

	/** Optional tags describing the context of the transaction. */
	FGameplayTagContainer* ContextTags = nullptr;

protected:
	//~ Begin FInventoryItemTransactionBase Interface
	virtual bool Undo() override
	{
		return false;
	}

	virtual bool Redo() override
	{
		return false;
	}
	//~ End FInventoryItemTransactionBase Interface
};