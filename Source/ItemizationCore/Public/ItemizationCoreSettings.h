// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "ItemizationCoreSettings.generated.h"

/** Configure the settings for the itemization core system. */
UCLASS(Config=Engine, DefaultConfig, MinimalAPI, DisplayName="Itemization Core")
class UItemizationCoreSettings : public UObject
{
	GENERATED_BODY()

public:
	UItemizationCoreSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	ITEMIZATIONCORE_API static const UItemizationCoreSettings* Get();
	ITEMIZATIONCORE_API  static UItemizationCoreSettings* GetMutable();

	UFUNCTION()
	static TArray<FName> GetItemTypes();

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	/** Tag, that if present on an item, will allow the item to stay in the inventory even if it's stack is empty. */
	UPROPERTY(Config, EditDefaultsOnly, Category=Traits)
	FGameplayTag AllowEmptyStackTag;

	/** Tag, that if present on an item, will hide the toast message when the item gets added to the inventory. */
	UPROPERTY(Config, EditDefaultsOnly, Category=Traits)
	FGameplayTag HideToastTag;

	/** Tag, that if present on an item, will prevent the item from being removed during RemoveAllInventoryItems. */
	UPROPERTY(Config, EditDefaultsOnly, Category=Traits)
	FGameplayTag IgnoreRemoveAllTag;

	/** Tag, that if present on an item, will count towards the inventory limit. */
	UPROPERTY(Config, EditDefaultsOnly, Category=Traits)
	FGameplayTag CountTowardsLimitTag;

	/** Tag, that if present on an item, will only allow a single stack of this item in the inventory list. */
	UPROPERTY(Config, EditDefaultsOnly, Category=Traits)
	FGameplayTag SingleStackTag;

	/** Tag, that if present on an item, will prevent the item from being saved in the inventory. (For test or runtime items) */
	UPROPERTY(Config, EditDefaultsOnly, Category=Traits)
	FGameplayTag TransientTag;

	/** Tag, that if present on an item, will automatically combine with other stacks in the inventory (if any) to fill larger stacks first, before creating a new stack. */
	UPROPERTY(Config, EditDefaultsOnly, Category=Traits)
	FGameplayTag AutoCombineStacks;

	/** Tag, that if present on an item, will force it into overflow and therefore not displaying in the inventory "grid". */
	UPROPERTY(Config, EditDefaultsOnly, Category=Traits)
	FGameplayTag ForceIntoOverflowTag;

	/** Tag, that if present on an item, will mark it as having durability and therefore will cause additional checks within the inventory.
	 * For instance item A can be stacked 2x but if one of them doesnt hav full durability, the can't be stacked.. */
	UPROPERTY(Config, EditDefaultsOnly, Category=Traits)
	FGameplayTag HasDurabilityTag;

	/** List of item types registered in the itemization core system and asset manager. */
	UPROPERTY(Config, EditDefaultsOnly, Category=ItemTypes)
	TArray<FName> ItemTypes;
};
