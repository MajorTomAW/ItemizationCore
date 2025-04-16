// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/ComponentData/ItemComponentData_MaxStackSize.h"

#include "Enums/EItemComponentInstancingPolicy.h"

FItemComponentData_MaxStackSize::FItemComponentData_MaxStackSize()
	: MaxStackSize(1.f)
{
	InstancingPolicy = EItemComponentInstancingPolicy::NonInstanced;
}

void FItemComponentData_MaxStackSize::EvaluateItemEntry(
	FInventoryItemEntry& ItemEntry,
	const FInventoryItemTransactionBase& Transaction) const
{
	const int32 Max = GetMaxStackSize();
}

int32 FItemComponentData_MaxStackSize::GetMaxStackSize() const
{
	return MaxStackSize.AsInteger();
}
