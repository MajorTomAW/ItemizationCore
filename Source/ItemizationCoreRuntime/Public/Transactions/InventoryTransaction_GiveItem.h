// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryItemTransactionBase.h"

#include "InventoryTransaction_GiveItem.generated.h"

struct FGameplayTagContainer;
class AInventoryBase;

USTRUCT(BlueprintType)
struct FInventoryTransaction_GiveItem : public FInventoryItemTransactionBase
{
	GENERATED_BODY()
	
public:
	FInventoryTransaction_GiveItem();
	FInventoryTransaction_GiveItem(AController* InInstigator, AInventoryBase* InTarget, int32 InDelta, FGameplayTagContainer* InContextTags = nullptr);

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