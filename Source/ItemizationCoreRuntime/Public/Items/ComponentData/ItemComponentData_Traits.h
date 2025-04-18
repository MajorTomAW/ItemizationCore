// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemComponentData.h"

#include "ItemComponentData_Traits.generated.h"

/** Item data for adding tagged traits to the item. */
USTRUCT(DisplayName = "Traits Item Data")
struct FItemComponentData_Traits : public FItemComponentData
{
	GENERATED_BODY()

	FItemComponentData_Traits();

public:
	/** Traits that this item has. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Traits, meta=(ExposeFunctionCategories="ItemizationCore.Item.Traits"))
	FGameplayTagContainer Traits;

protected:
	//~ Begin FItemComponentData Interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End FItemComponentData Interface
};

/** Wrapper for this traits item data, just cor convenience :P */
typedef FItemComponentData_Traits FItemTraitsData;
