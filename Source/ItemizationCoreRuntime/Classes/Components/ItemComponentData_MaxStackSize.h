// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "Components/ItemComponentData.h"

#include "ItemComponentData_MaxStackSize.generated.h"

/**
 * Item data for modifying maximum stack size for the item.
 * Without this item data, the max stack size is assumed to be 1.
 */
USTRUCT(meta = (CosmeticComponent = false, SingletonComponent = true), DisplayName = "Max Stack Size Item Data")
struct FItemComponentData_MaxStackSize: public FItemComponentData
{
	GENERATED_BODY()
	FItemComponentData_MaxStackSize();

public:
	/** If the value is less than or equal to 0, the item will have an infinite maximum stack size. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Max Stack Size")
	FScalableFloat MaxStackSize;

protected:
	//~ Begin FItemComponentData Interface
	virtual void EvaluateContext(const FInventoryItemEntry& ItemEntry, FItemActionContextData& Context) const override;
	virtual bool CanCombineItemStacks(const FInventoryItemEntry& ThisEntry, const FInventoryItemEntry& OtherEntry, FItemActionContextData& Context) const override;
	//~ End FItemComponentData Interface
};
