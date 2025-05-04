// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryItemTransactionBase.generated.h"

class UObject;
class AController;

/**
 * Struct defining a base transactable block for inventory items.
 * This is used to pass data between different functions or server and client of predictive behavior.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventoryItemTransactionBase
{
	GENERATED_BODY()

	FInventoryItemTransactionBase() = default;
	FInventoryItemTransactionBase(AController* InInstigator);
	virtual ~FInventoryItemTransactionBase() = default;

	/** Called to undo/revert the transaction. */
	virtual bool Undo();

	/** Called to redo the transaction. */
	virtual bool Redo();

public:
	/** Instigator controller that "owns" this transaction. */
	TWeakObjectPtr<AController> Instigator;

	/** Stores the transaction index, which is used to identify the transaction. */
	int32 Index = INDEX_NONE;
};