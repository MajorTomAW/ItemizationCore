// Copyright © 2025 Playton. All Rights Reserved.


#include "Items/Data/ItemComponentData_MaxStackSize.h"

#include "Items/InventoryItemEntry.h"

FItemComponentData_MaxStackSize::FItemComponentData_MaxStackSize()
{
}

int32 FItemComponentData_MaxStackSize::GetMaxStackSize() const
{
	return MaxStackSize.AsInteger();
}

void FItemComponentData_MaxStackSize::EvaluateItemEntry(FInventoryOp_ItemAction::FParams& Params) const
{
	Params.ItemEntry->SetStatValue(Itemization::Tags::TAG_ItemStat_MaxStackSize, GetMaxStackSize());
}
