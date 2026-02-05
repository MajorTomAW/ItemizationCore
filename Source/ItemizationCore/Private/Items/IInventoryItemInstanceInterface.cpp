// Author: Tom Werner (MajorT), 2026

#include "Items/IInventoryItemInstanceInterface.h"

#include "Items/ItemDefinitionBase.h"

const UItemDefinitionBase* IInventoryItemInstanceInterface::GetItemDefinition() const
{
	if (const auto* ItemEntry = GetItemEntry())
	{
		return ItemEntry->GetItemDefinition();
	}

	return nullptr;
}
