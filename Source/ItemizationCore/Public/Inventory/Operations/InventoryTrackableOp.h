// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryTrackableOp.generated.h"

class UObject;
class AController;

/**
 * Struct defining a base transactable block for inventory items.
 * This is used to pass data between different functions or server and client of predictive behavior.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORE_API FInventoryTrackableOp
{
	GENERATED_BODY()

	FInventoryTrackableOp() = default;
	FInventoryTrackableOp(AController* InInstigator)
		: Instigator(InInstigator)
	{
	}
	
	virtual ~FInventoryTrackableOp() = default;

	/** Called to undo/revert the transaction. */
	virtual bool Undo() { return false; }

	/** Called to redo the transaction. */
	virtual bool Redo() { return false; }

public:
	/** Instigator controller that "owns" this transaction. */
	TWeakObjectPtr<AController> Instigator;

	/** Stores the transaction index, which is used to identify the transaction. */
	int32 Index = INDEX_NONE;
};