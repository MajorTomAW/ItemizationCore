// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ItemComponentData.h"

#include "ItemComponentData_Traits.generated.h"

/**
 * Item data for adding tagged traits to the item.
 */
USTRUCT(DisplayName = "Traits Item Data", meta = (SingletonComponent = true))
struct FItemComponentData_Traits : public FItemComponentData
{
	GENERATED_BODY()
	FItemComponentData_Traits();

public:
	/** Traits that this item has */
	UPROPERTY(EditAnywhere, Category = "Traits", meta = (Categories = "ItemizationCore.Item.Trait"))
	FGameplayTagContainer Traits;

protected:
	//~ Begin FItemComponentData Interface
	virtual bool CanCreateNewStack(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& Context) const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End FItemComponentData Interface
};