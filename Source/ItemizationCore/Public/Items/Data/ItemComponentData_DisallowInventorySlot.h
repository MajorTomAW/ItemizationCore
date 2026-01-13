// Author: Tom Werner (MajorT), 2025

#pragma once

#include "GameplayTagContainer.h"
#include "ItemComponentData.h"


#include "ItemComponentData_DisallowInventorySlot.generated.h"

/**
 * Item component data for disallowing the item to be placed in a specific inventory slot
 */
USTRUCT(DisplayName="Disallow Slow Item Data")
struct FItemComponentData_DisallowInventorySlot : public FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData_DisallowInventorySlot();

public:
	/** If the slot has any of these tags, the item can't be placed in it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Tags)
	FGameplayTagContainer DisallowedSlotTags;

protected:
	//~ Begin FItemComponentData Interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
	virtual FText GetDescription() const override;
#endif
	//~ End FItemComponentData Interface
};
