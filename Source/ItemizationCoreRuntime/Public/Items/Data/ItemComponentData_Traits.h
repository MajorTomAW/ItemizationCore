// Author: Tom Werner (MajorT), 2025

#pragma once

#include "GameplayTagContainer.h"
#include "ItemComponentData.h"

#include "ItemComponentData_Traits.generated.h"

class UItemDefinitionBase;
class UObject;

/** Item data for adding tagged traits to the item. */
USTRUCT(DisplayName="Traits Item Data")
struct FItemComponentData_Traits : public FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData_Traits();
	static bool HasTrait(const UItemDefinitionBase* InItemDefinition, const FGameplayTag& TraitToCheck);

public:
	/** Traits that this item has. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Traits, meta=(Categories="Item.Trait"))
	FGameplayTagContainer Traits;

protected:
	//~ Begin FItemComponentData Interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End FItemComponentData Interface
};