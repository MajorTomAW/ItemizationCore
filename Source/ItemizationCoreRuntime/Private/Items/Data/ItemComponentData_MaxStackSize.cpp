// Author: Tom Werner (MajorT), 2025


#include "Items/Data/ItemComponentData_MaxStackSize.h"

#include "ItemizationGameplayTags.h"
#include "Items/InventoryItemEntry.h"

FItemComponentData_MaxStackSize::FItemComponentData_MaxStackSize()
{
}

int32 FItemComponentData_MaxStackSize::GetMaxStackSize() const
{
	return MaxStackSize.AsInteger();
}

void FItemComponentData_MaxStackSize::EvaluateItemEntry(
	FInventoryItemEntry& ItemEntry,
	const FInventoryTrackableOp& Transaction) const
{
	ItemEntry.SetStatValue(Itemization::Tags::TAG_ItemStat_MaxStackSize, GetMaxStackSize());
}
