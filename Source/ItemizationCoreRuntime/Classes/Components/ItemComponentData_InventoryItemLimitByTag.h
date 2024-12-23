// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "Components/ItemComponentData.h"

#include "ItemComponentData_InventoryItemLimitByTag.generated.h"

/**
 * Item data for defining the limited inventory amount for items given a Tag.
 * This can be any tag you wish to define to group items and limit their amount in the inventory.
 * When adding a new item to the inventory, if the amount of individual items that use this same LimitTag definition exceeds the LimitAmount value,
 * we remove the first (FIFO) to clear space for the new item.
 */
USTRUCT(meta = (SingletonComponent = true), DisplayName = "Inventory Item Limit By Tag")
struct FItemComponentData_InventoryItemLimitByTag : public FItemComponentData
{
	GENERATED_BODY()
	FItemComponentData_InventoryItemLimitByTag();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FScalableFloat LimitAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FGameplayTag LimitTag;

protected:
	//~ Begin FItemComponentData Interface
	virtual void EvaluateContext(const FInventoryItemEntry& ItemEntry, FItemActionContextData& Context) const override;
	virtual bool CanCreateNewStack(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& Context) const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End FItemComponentData Interface
};