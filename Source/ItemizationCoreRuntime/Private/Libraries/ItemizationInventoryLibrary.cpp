// Author: Tom Werner (MajorT), 2025


#include "Libraries/ItemizationInventoryLibrary.h"

#include "Interfaces/InventoryOwnerInterface.h"
#include "Inventory/InventoryBase.h"
#include "Transactions/InventoryTransaction_GiveRemoveItem.h"

FInventoryItemHandle UItemizationInventoryLibrary::GiveItem(
	TScriptInterface<IInventoryOwnerInterface> InventoryOwner,
	UItemDefinitionBase* ItemDefinition,
	int32 StackCount, int32& Excess)
{
	if (AInventoryBase* Inventory = InventoryOwner->GetInventory())
	{
		FInventoryItemEntry ItemEntry(ItemDefinition, StackCount, InventoryOwner.GetObject());

		FInventoryTransaction_GiveRemoveItem Transaction;
		Transaction.Delta =	StackCount;
		Transaction.TargetInventory = Inventory;
		
		return Inventory->GiveItem(ItemEntry, Excess, Transaction);
	}

	return FInventoryItemHandle();
}

void UItemizationInventoryLibrary::PerformTransaction(
	TScriptInterface<IInventoryOwnerInterface> InventoryOwner,
	FInventoryTrackableOp& Transaction)
{
	if (AInventoryBase* Inventory = InventoryOwner->GetInventory())
	{
		
	}
}
