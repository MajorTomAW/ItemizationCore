// Author: Tom Werner (MajorT), 2025


#include "Transactions/InventoryTransaction_GiveItem.h"

#include "Inventory/InventoryBase.h"


FInventoryTransaction_GiveItem::FInventoryTransaction_GiveItem()
{
}

FInventoryTransaction_GiveItem::FInventoryTransaction_GiveItem(
	AController* InInstigator,
	AInventoryBase* InTarget,
	int32 InDelta,
	FGameplayTagContainer* InContextTags)
		: FInventoryItemTransactionBase(InInstigator)
	, TargetInventory(InTarget)
	, Delta(InDelta)
	, ContextTags(InContextTags)
{
}

bool FInventoryTransaction_GiveItem::Undo()
{
	//@TODO
	return false;
}

bool FInventoryTransaction_GiveItem::Redo()
{
	//@TODO
	return false;
}
