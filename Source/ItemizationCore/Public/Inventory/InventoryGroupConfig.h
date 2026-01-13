// Author: Tom Werner (MajorT), 2025

#pragma once

#include "GameplayTagContainer.h"

#include "InventoryGroupConfig.generated.h"

USTRUCT(BlueprintType)
struct FInventoryGroupConfig
{
	GENERATED_BODY()

	FInventoryGroupConfig() = default;

public:
	UPROPERTY(EditAnywhere, Category = InventoryGroupConfig, meta=(Categories="Inventory.Group"))
	FGameplayTag GroupType;

	UPROPERTY(EditAnywhere, Category = InventoryGroupConfig)
	uint32 NumItemRows = 0;

	UPROPERTY(EditAnywhere, Category = InventoryGroupConfig)
	uint32 NumItemColumns = 0;

	UPROPERTY(EditAnywhere, Category = InventoryGroupConfig)
	TMap<uint32, FGameplayTagContainer> SlotTagMap;
};
