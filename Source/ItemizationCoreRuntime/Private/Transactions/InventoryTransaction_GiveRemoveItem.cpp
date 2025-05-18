// Author: Tom Werner (MajorT), 2025


#include "Transactions/InventoryTransaction_GiveRemoveItem.h"

#include "Inventory/InventoryBase.h"


FInventoryTransaction_GiveRemoveItem::FInventoryTransaction_GiveRemoveItem()
{
}

FInventoryTransaction_GiveRemoveItem::FInventoryTransaction_GiveRemoveItem(
	AController* InInstigator,
	AInventoryBase* InTarget,
	int32 InDelta,
	FGameplayTagContainer* InContextTags)
		: FInventoryTrackableOp(InInstigator)
	, TargetInventory(InTarget)
	, Delta(InDelta)
	, ContextTags(InContextTags)
{
}

bool FInventoryTransaction_GiveRemoveItem::Undo()
{
	//@TODO
	return false;
}

bool FInventoryTransaction_GiveRemoveItem::Redo()
{
	//@TODO
	return false;
}
