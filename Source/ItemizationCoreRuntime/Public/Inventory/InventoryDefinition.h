// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryProperties.h"
#include "Engine/DataAsset.h"

#include "InventoryDefinition.generated.h"

/** Class defining a single inventory in the game. */
UCLASS(BlueprintType, Const, MinimalAPI)
class UInventoryDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UInventoryDefinition()
	{
	}
	
	/** List of inventories to be created. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	TArray<FInventoryProperties> InventoryList;

	/** List of equippable inventories to be created. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	TArray<FEquippableInventoryProperties> EquippableInventoryList;

	/** List of optional tags that can be used to identify this inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	FGameplayTagContainer GameplayTags;
};
