// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ItemComponentData.h"

#include "ItemComponentData_SlotSize.generated.h"

/**
 * Item data for changing the number of inventory slots occupied by an item.
 * Without this item data, the slot size is assumed to be 1.
 */
USTRUCT(meta = (CosmeticComponent = false, SingletonComponent = true), DisplayName = "Slot Size Item Data")
struct FItemComponentData_SlotSize : public FItemComponentData
{
	GENERATED_BODY()
	FItemComponentData_SlotSize();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slot Size")
	uint8 SlotSize;

protected:
	//~ Begin FItemComponentData Interface
	virtual void EvaluateContext(const FInventoryItemEntry& ItemEntry, FItemActionContextData& Context) const override;
	//~ End FItemComponentData Interface
};
