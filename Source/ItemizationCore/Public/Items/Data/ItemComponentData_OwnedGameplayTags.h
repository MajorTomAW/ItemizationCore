// Author: Tom Werner (MajorT), 2025

#pragma once
#include "GameplayTagContainer.h"
#include "ItemComponentData.h"


#include "ItemComponentData_OwnedGameplayTags.generated.h"

/**
 * Item data for adding gameplay tags owned by the item.
 */
USTRUCT(DisplayName="Owned Gameplay Tags Item Data")
struct FItemComponentData_OwnedGameplayTags : public FItemComponentData
{
	GENERATED_BODY()

public:
	/** Tags owned by the item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Tags)
	FGameplayTagContainer Tags;

protected:
	//~ Begin FItemComponentData Interface
#if WITH_EDITOR
	virtual FText GetDescription() const override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End FItemComponentData Interface
};
