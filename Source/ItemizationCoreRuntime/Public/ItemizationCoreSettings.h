// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "ItemizationCoreSettings.generated.h"

/** Configure the settings for the itemization core system. */
UCLASS(Config = Engine, DefaultConfig, DisplayName="Itemization Core", MinimalAPI)
class UItemizationCoreSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UItemizationCoreSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	ITEMIZATIONCORERUNTIME_API static UItemizationCoreSettings* Get();

public:
	/** List of item types that are also being registered as primary assets. */
	UPROPERTY(Config, EditAnywhere, Category = Items)
	TArray<FName> ItemTypes;

	/** Tag, that if present on an item, will allow the item to stay in the inventory even if it's stack is empty. */
	UPROPERTY(Config, EditDefaultsOnly, Category=InventorySystem, meta=(ConfigRestartRequired=true))
	FGameplayTag AllowEmptyStackTag;

	/** Tag, that if present on an item, will hide the toast message when the item gets added to the inventory. */
	UPROPERTY(Config, EditDefaultsOnly, Category=InventorySystem, meta=(ConfigRestartRequired=true))
	FGameplayTag HideToastTag;

	/** Tag, that if present on an item, will prevent the item from being removed during RemoveAllInventoryItems. */
	UPROPERTY(Config, EditDefaultsOnly, Category=InventorySystem, meta=(ConfigRestartRequired=true))
	FGameplayTag IgnoreRemoveAllTag;

	/** Tag, that if present on an item, will count towards the inventory limit. */
	UPROPERTY(Config, EditDefaultsOnly, Category=InventorySystem, meta=(ConfigRestartRequired=true))
	FGameplayTag CountTowardsLimitTag;

	/** Tag, that if present on an item, will only allow a single stack of this item in the inventory list. */
	UPROPERTY(Config, EditDefaultsOnly, Category=InventorySystem, meta=(ConfigRestartRequired=true))
	FGameplayTag SingleStackTag;

	/** Tag, that if present on an item, will prevent the item from being saved in the inventory. (For test or runtime items) */
	UPROPERTY(Config, EditDefaultsOnly, Category=InventorySystem, meta=(ConfigRestartRequired=true))
	FGameplayTag TransientTag;
};
