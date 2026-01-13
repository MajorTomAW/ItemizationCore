// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "ItemComponentData.h"

#include "ItemComponentData_PlayerGrantedTags.generated.h"

/** Item data for granting tags to the owning player when added to the inventory. */
USTRUCT(DisplayName = "Player Granted Tags Data")
struct FItemComponentData_PlayerGrantedTags : public FItemComponentData
{
	GENERATED_BODY()

protected:
	/** Traits that this item has. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Traits)
	FGameplayTagContainer TagsToGrant;

	/** If true, will add to replicated tag list instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Traits)
	bool bShouldReplicate = false;

protected:
	//~ Begin FItemComponentData Interface
	virtual void OnItemGiven(FInventoryItemEntry& ItemEntry, const FInventoryHandle& InventoryHandle) const override;
	virtual void OnItemRemoved(FInventoryItemEntry& ItemEntry, const FInventoryHandle& InventoryHandle) const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
	virtual FText GetDescription() const override;
#endif
	//~ End FItemComponentData Interface
};
