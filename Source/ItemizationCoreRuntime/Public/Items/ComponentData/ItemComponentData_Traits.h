// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemComponentData.h"

#include "ItemComponentData_Traits.generated.h"

class UItemDefinition;
/** Item data for adding tagged traits to the item. */
USTRUCT(DisplayName = "Traits Item Data")
struct FItemComponentData_Traits : public FItemComponentData
{
	GENERATED_BODY()

	FItemComponentData_Traits();
	static bool HasTrait(const UItemDefinition* ItemDefinition, const FGameplayTag& Trait);

public:
	/** Traits that this item has. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Traits, meta=(ExposeFunctionCategories="Item.Trait"))
	FGameplayTagContainer Traits;

protected:
	//~ Begin FItemComponentData Interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End FItemComponentData Interface
};
