// Author: Tom Werner (MajorT), 2025

#pragma once

#include "ItemComponentData.h"
#include "ScalableFloat.h"

#include "ItemComponentData_MaxStackSize.generated.h"

/**
 * Item data for modifying the maximum stack size for the item.
 * Without this item data, the max stack size is assumed to be 1.
 */
USTRUCT(DisplayName="Max Stack Size Item Data")
struct FItemComponentData_MaxStackSize : public FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData_MaxStackSize();

public:
	/** If this value is less than or equal to 0, the item will have an infinite maximum stack size. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = MaxStackSize)
	FScalableFloat MaxStackSize;

	/** Returns the max stack size for this item data. */
	int32 GetMaxStackSize() const;

protected:
	//~ Begin FItemComponentData Interface
	virtual void EvaluateItemEntry(FInventoryItemEntry& ItemEntry, const FInventoryTrackableOp& Transaction) const override;
	//~ End FItemComponentData Interface
};