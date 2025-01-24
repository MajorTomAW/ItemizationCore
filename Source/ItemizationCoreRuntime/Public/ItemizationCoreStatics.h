// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "ItemizationCoreStatics.generated.h"


struct FItemActionContextData;
struct FInventoryItemEntryHandle;
class UInventoryManager;
class UInventoryItemInstance;
class UItemDefinition;

/**
 * Itemization statics library.
 */
UCLASS(meta = (ScriptName = "ItemizationCoreStatics"))
class ITEMIZATIONCORERUNTIME_API UItemizationCoreStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Converts an item handle to a string. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "To String (Item Handle)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Utilities|String")
	static FString Conv_ItemHandleToString(const FInventoryItemEntryHandle& Handle);

	/*/** Converts an item context to a string. #1#
	UFUNCTION(BlueprintPure, meta = (DisplayName = "To String (Item Context Data)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Utilities|String")
	static FString Conv_ItemContextToString(const FItemActionContextData& ContextData);*/

	/** Equality operator for two item handles. */
	UFUNCTION(BlueprintPure, Category = "Itemization Core|Item Handle", meta = (DisplayName = "Equal (Item Handle)", CompactNodeTitle = "==", ScriptOperator = "=="))
	static bool EqualEqual_ItemHandle(const FInventoryItemEntryHandle& A, const FInventoryItemEntryHandle& B);

	/** Inequality operator for two item handles. */
	UFUNCTION(BlueprintPure, Category = "Itemization Core|Item Handle", meta = (DisplayName = "Not Equal (Item Handle)", CompactNodeTitle = "!=", ScriptOperator = "!="))
	static bool NotEqual_ItemHandle(const FInventoryItemEntryHandle& A, const FInventoryItemEntryHandle& B);

	/**
	 * Provides the inventory item instance associated with an item handle.
	 *
	 * @param InventoryManager The inventory manager that owns the item.
	 * @param ItemHandle The handle of the item to get the instance of.
	 * @returns The item instance associated with the handle or nullptr if the item couldn't be found.
	 */
	UFUNCTION(BlueprintPure, Category = "Itemization Core|Item Handle", meta = (DisplayName = "Get Item Instance From Handle"))
	static const UInventoryItemInstance* GetItemInstanceFromHandle(UInventoryManager* InventoryManager, const FInventoryItemEntryHandle& ItemHandle);


	/** Attempts to find an icon for the given item definition. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item Definition")
	static TSoftObjectPtr<UTexture2D> GetItemIcon(const UItemDefinition* ItemDefinition);
};
