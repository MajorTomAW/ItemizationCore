// Author: Tom Werner (MajorT), 2025 November


#include "Items/Data/ItemComponentData_MaxStackSize.h"

#include "Items/InventoryItemEntry.h"

FItemComponentData_MaxStackSize::FItemComponentData_MaxStackSize()
{
}

int32 FItemComponentData_MaxStackSize::GetMaxStackSize() const
{
	return MaxStackSize.AsInteger();
}

#if WITH_EDITOR
FText FItemComponentData_MaxStackSize::GetDescription() const
{
	return FText::Format(INVTEXT("Max Stack Size: {0}"), GetMaxStackSize());
}
#endif
