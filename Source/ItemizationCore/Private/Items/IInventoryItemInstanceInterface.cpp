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

const UItemDefinitionBase* IInventoryItemInstanceInterface::GetItemDefinition_Typed(
	TSubclassOf<UItemDefinitionBase> ItemType) const
{
	if (const UItemDefinitionBase* ItemDefinition = GetItemDefinition())
	{
		if (ItemDefinition->IsA(ItemType))
		{
			return ItemDefinition;
		}
	}

	return nullptr;
}
