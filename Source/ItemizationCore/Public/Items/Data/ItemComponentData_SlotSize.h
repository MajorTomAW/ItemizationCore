// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "ItemComponentData.h"

#include "ItemComponentData_SlotSize.generated.h"

/**
 * Item data for setting the amount of slots this item eats up.
 * Without this item data, the slot size is assumed to be 1.
 */
USTRUCT(DisplayName="Slot Size Item Data")
struct FItemComponentData_SlotSize : public FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData_SlotSize();

public:
	/** */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = SlotSize)
	uint8 SlotSize;

protected:
	//~ Begin FItemComponentData Interface
	virtual void EvaluateItemEntry(FInventoryOp_GiveAction::FParams& Params) const override;

#if WITH_EDITOR
	virtual FText GetDescription() const override;
#endif
	//~ End FItemComponentData Interface
};
