// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryTrackableOp.h"

#include "InventoryTransaction_GiveRemoveItem.generated.h"

struct FGameplayTagContainer;
class AInventoryBase;

USTRUCT(BlueprintType)
struct FInventoryTransaction_GiveRemoveItem : public FInventoryTrackableOp
{
	GENERATED_BODY()
	
public:
	FInventoryTransaction_GiveRemoveItem();
	FInventoryTransaction_GiveRemoveItem(AController* InInstigator, AInventoryBase* InTarget, int32 InDelta, FGameplayTagContainer* InContextTags = nullptr);

	/** The target inventory that we want to give the item to. */
	TWeakObjectPtr<AInventoryBase> TargetInventory = nullptr;

	/** The delta number of items to give/remove. */
	int32 Delta = 0;

	/** Optional tags describing the context of the transaction. */
	FGameplayTagContainer* ContextTags = nullptr;

protected:
	//~ Begin FInventoryItemTransactionBase Interface
	virtual bool Undo() override;
	virtual bool Redo() override;
	//~ End FInventoryItemTransactionBase Interface
};