// Author: Tom Werner (MajorT), 2025 November


#include "Items/Data/ItemComponentData_SlotSize.h"

#include "Items/InventoryItemEntry.h"

FItemComponentData_SlotSize::FItemComponentData_SlotSize()
	: SlotSize(1)
{
}

void FItemComponentData_SlotSize::EvaluateItemEntry(FInventoryOp_GiveAction::FParams& Params) const
{
	//@TODO: Inject slot size ?Params.ItemEntry->SetStatValue(Itemization::Tags::)
}

#if WITH_EDITOR
FText FItemComponentData_SlotSize::GetDescription() const
{
	return FText::Format(INVTEXT("Slot Size: {0}"), SlotSize);
}
#endif
