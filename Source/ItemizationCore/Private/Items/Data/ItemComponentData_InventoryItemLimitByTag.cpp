// Author: Tom Werner (MajorT), 2025


#include "Items/Data/ItemComponentData_InventoryItemLimitByTag.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemComponentData_InventoryItemLimitByTag)

FItemComponentData_InventoryItemLimitByTag::FItemComponentData_InventoryItemLimitByTag()
{
}

void FItemComponentData_InventoryItemLimitByTag::PostItemEntryCreated(FInventoryItemEntry& ItemEntry) const
{
	Super::PostItemEntryCreated(ItemEntry);
}

#if WITH_EDITOR
FText FItemComponentData_InventoryItemLimitByTag::GetDescription() const
{
	return FText::Format(INVTEXT("Inventory Limit {0} with t ag {1}"),
		LimitAmount.AsInteger(), FText::FromString(LimitTag.ToString()));
}

EDataValidationResult FItemComponentData_InventoryItemLimitByTag::IsDataValid(FDataValidationContext& Context) const
{
	return Super::IsDataValid(Context);
}
#endif
