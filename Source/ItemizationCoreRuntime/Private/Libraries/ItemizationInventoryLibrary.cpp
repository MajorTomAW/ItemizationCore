// Author: Tom Werner (MajorT), 2025


#include "Libraries/ItemizationInventoryLibrary.h"

#include "Interfaces/InventoryOwnerInterface.h"
#include "Inventory/InventoryBase.h"
#include "Transactions/InventoryTransaction_GiveItem.h"

FInventoryItemHandle UItemizationInventoryLibrary::GiveItem(
	TScriptInterface<IInventoryOwnerInterface> InventoryOwner,
	UItemDefinitionBase* ItemDefinition,
	int32 StackCount)
{
	if (AInventoryBase* Inventory = InventoryOwner->GetInventory())
	{
		FInventoryItemEntry ItemEntry(ItemDefinition, StackCount, InventoryOwner.GetObject());
		int32 OutExcess;

		FInventoryTransaction_GiveItem Transaction;
		Transaction.Delta =	StackCount;
		Transaction.TargetInventory = Inventory;
		
		return Inventory->GiveItem(ItemEntry, OutExcess, Transaction);
	}

	return FInventoryItemHandle();
}
