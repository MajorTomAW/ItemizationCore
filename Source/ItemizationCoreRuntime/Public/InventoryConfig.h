// Author: Tom Werner (MajorT), 2025

#pragma once

#include "GameplayTagContainer.h"
#include "InventoryGroupConfig.h"

#include "InventoryConfig.generated.h"

/** Config asset for a single inventory. */
UCLASS(BlueprintType, Const, MinimalAPI)
class UInventoryConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UInventoryConfig()
		: bIsPersistent(false)
	{
	}

	/** The name or identifier that can be later used to identify this inventory. */
	UPROPERTY(EditAnywhere, Category = InventoryConfig)
	FGameplayTag InventoryTag;

	/** Whether this inventory should be persistent across game sessions. */
	UPROPERTY(EditAnywhere, Category = InventoryConfig)
	uint8 bIsPersistent : 1;

	/** List of individual inventory "groups" that can be used to group inventory slots together. */
	UPROPERTY(EditAnywhere, Category = InventoryConfig)
	TArray<FInventoryGroupConfig> InventoryGroupConfigs;

protected:
	//~ Begin UObject Interface
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
	//~ End UObject Interface
};