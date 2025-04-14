// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/InventoryProperties.h"

#include "Inventory/EquippableInventory.h"
#include "Inventory/Inventory.h"
#include "Inventory/SlottableInventory.h"

FInventoryPropertiesBase::FInventoryPropertiesBase()
	: TotalSlotsOverride(INDEX_NONE)
{
}

FInventoryProperties::FInventoryProperties()
	: InventoryClass(AInventory::StaticClass())
{
}

FSlottableInventoryProperties::FSlottableInventoryProperties()
	: InventoryClass(ASlottableInventory::StaticClass())
{
}

FEquippableInventoryProperties::FEquippableInventoryProperties()
	: InventoryClass(AEquippableInventory::StaticClass())
{
}
