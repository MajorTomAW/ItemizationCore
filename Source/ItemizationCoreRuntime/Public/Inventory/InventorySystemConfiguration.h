// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventorySystemConfiguration.generated.h"

class UInputMappingContext;
class UInventoryDefinition;

/** Class that stores all the required information to create an inventory system for an actor. */
UCLASS(BlueprintType, Const, MinimalAPI)
class UInventorySystemConfiguration : public UDataAsset
{
	GENERATED_BODY()

public:
	UInventorySystemConfiguration()
	{
	}

	/** The inventory definition to use for a player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setup)
	TObjectPtr<UInventoryDefinition> PlayerInventorySetup;

	/** The inventory definition to use for a bot-controlled pawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Setup)
	TObjectPtr<UInventoryDefinition> AIInventorySetup;

	/** The input mapping context to assign to the player inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TSoftObjectPtr<UInputMappingContext> InventoryInputMappings;
};
