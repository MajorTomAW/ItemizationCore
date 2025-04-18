// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemComponentData.h"

#include "ItemComponentData_OwnedGameplayTags.generated.h"

/** Item data for adding gameplay tags owned by the item. */
USTRUCT(DisplayName = "Owned Gameplay Tags Item Data")
struct FItemComponentData_OwnedGameplayTags : public FItemComponentData
{
	GENERATED_BODY()

	FItemComponentData_OwnedGameplayTags();

public:
	/** Tags that this item has. */
	UPROPERTY(EditDefaultsOnly, Category=GameplayTags)
	FGameplayTagContainer Tags;
	
protected:
	//~ Begin FItemComponentData Interface
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;
	virtual void EvaluateItemEntry(FInventoryItemEntry& ItemEntry, const FInventoryItemTransactionBase& Transaction) const override;
	//~ End FItemComponentData Interface
};