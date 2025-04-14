// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventorySetupData.generated.h"

class UInputMappingContext;
class UInventoryDefinition;
class UUserWidget;

/**
 * Setup data object storing information about an entire inventory system.
 */
UCLASS(BlueprintType, Const)
class ITEMIZATIONCORERUNTIME_API UInventorySetupData : public UDataAsset
{
	GENERATED_BODY()

public:
	UInventorySetupData();

	/** List of inventory definitions to create. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = General)
	TArray<TSoftObjectPtr<UInventoryDefinition>> InventoryDefinitions;

	/** Mapping context to assign to the primary player that can be used for slot key binds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TSoftObjectPtr<UInputMappingContext> InputMappingContext;

	/** Optional inventory ui widget class to use for displaying the inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI, DisplayName="Inventory UI Widget Class")
	TSoftClassPtr<UUserWidget> InventoryUIWidgetClass;

	/** Optional inventory hud widget class displaying the inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI, DisplayName="Inventory HUD Widget Class")
	TSoftClassPtr<UUserWidget> InventoryHUDWidgetClass;
};
