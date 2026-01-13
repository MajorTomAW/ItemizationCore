// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "ItemizationItemFunctionLibrary.generated.h"

#define MY_API ITEMIZATIONCORE_API

class UItemDefinitionBase;

/** Blueprint-Library for inventory items. Some function might be useful to call natively too. */
UCLASS(meta=(ScriptName="ItemizationItemLibrary"), MinimalAPI)
class UItemizationItemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns true if the item definition has a "TraitsItemData" which contains the given trait. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Itemization|Item", meta=(Categories="Item.Trait"))
	static MY_API bool ItemHasTrait(const UItemDefinitionBase* ItemDefinition, FGameplayTag TraitToCheck);

	/** Returns true if the item definition can be stacked multiple times in a single slot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Itemization|Item")
	static MY_API bool IsItemStackable(const UItemDefinitionBase* ItemDefinition);

	/** Returns the maximum stack size for the item definition. If the item is not stackable, it returns 1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Itemization|Item")
	static MY_API int32 GetItemMaxStackSize(const UItemDefinitionBase* ItemDefinition);

	/** Returns the icon texture for the item definition. If no icon is set, it returns a null pointer. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Itemization|Item")
	static MY_API TSoftObjectPtr<UTexture2D> GetItemIcon(const UItemDefinitionBase* ItemDefinition);

	/** Returns the item name as a plain text. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Itemization|Item")
	static MY_API FText GetItemName(const UItemDefinitionBase* ItemDefinition, bool bUsePlural = false);

	/** Returns the item description as a plain text. If bFallbackToShort is true, it will return the short description if the long one is not set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Itemization|Item")
	static MY_API FText GetItemDescription(const UItemDefinitionBase* ItemDefinition, bool bFallbackToShort = true);

	/** Returns the item description as a rich text. If bFallbackToShort is true, it will return the short description if the long one is not set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Itemization|Item")
	static MY_API FText GetItemRichDescription(const UItemDefinitionBase* ItemDefinition, bool bFallbackToShort = true);

	/** Returns the item type as a text. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Itemization|Item")
	static MY_API FText GetItemTypeName(const UItemDefinitionBase* ItemDefinition);
};

#undef MY_API
