// Author: Tom Werner (MajorT), 2025

#pragma once
#include "GameplayTagContainer.h"
#include "ItemComponentData.h"
#include "ScalableFloat.h"


#include "ItemComponentData_InventoryItemLimitByTag.generated.h"

/**
 * Item data for defining the limited inventory amount for items given a Tag.
 * This can be any tag you wish to define to group items and limit their amount in the inventory.
 * When adding a new item to the inventory, if the amount of individual items that use this same LimitTag definition exceeds the LimitAmount value,
 * we remove the first (FIFO) to clear space for the new item.
 */
USTRUCT(DisplayName="Inventory Item Limit By Tag")
struct FItemComponentData_InventoryItemLimitByTag : public FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData_InventoryItemLimitByTag();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Limit)
	FScalableFloat LimitAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Limit)
	FGameplayTag LimitTag;

protected:
	//~ Begin FItemComponentData Interface
	virtual void EvaluateItemEntry(FInventoryOp_GiveAction::FParams& Params) const override;

#if WITH_EDITOR
	virtual FText GetDescription() const override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End FItemComponentData Interface
};
