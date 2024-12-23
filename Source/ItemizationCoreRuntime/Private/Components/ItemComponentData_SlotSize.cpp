// Copyright Epic Games, Inc. All Rights Reserved.


#include "Components/ItemComponentData_SlotSize.h"


FItemComponentData_SlotSize::FItemComponentData_SlotSize()
{
	SlotSize = 1;
}

void FItemComponentData_SlotSize::EvaluateContext(const FInventoryItemEntry& ItemEntry, FItemActionContextData& Context) const
{
	Context.SlotSize = SlotSize;
}
