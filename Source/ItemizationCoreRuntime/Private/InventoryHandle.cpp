// Author: Tom Werner (MajorT), 2025


#include "InventoryHandle.h"

#include "Inventory/InventoryBase.h"

FGuid FInventoryHandle::AssignInventory(AInventoryBase* InInventory)
{
	checkf(!Guid.IsValid() && !Inventory.IsValid()
		, TEXT("Cannot assign inventory to a handle that already has one!"));

	check(InInventory);

	Inventory = InInventory;
	Guid = FGuid::NewGuid();
	return Guid;
}
