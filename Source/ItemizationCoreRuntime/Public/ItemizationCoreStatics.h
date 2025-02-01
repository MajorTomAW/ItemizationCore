// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinition.h"
#include "Components/ItemComponentData.h"
#include "Components/ItemComponentData_Ability.h"
#include "Components/ItemComponentData_Equipment.h"
#include "Components/ItemComponentData_Icon.h"
#include "Components/ItemComponentData_InventoryItemLimitByTag.h"
#include "Components/ItemComponentData_MaxStackSize.h"
#include "Components/ItemComponentData_OwnedGameplayTags.h"
#include "Components/ItemComponentData_SlotSize.h"
#include "Components/ItemComponentData_Traits.h"
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

	// --------------------------------------------------------------------------------------------
	// Item Component Data
	// @TODO: Make a custom K2Node for this so that we don't have to manually add new functions for each component type
	// --------------------------------------------------------------------------------------------

	/** Returns the ability component data for the specified item definition. */
	UFUNCTION(BlueprintPure, Category = "Itemization Core|Item Definition|Components", meta = (DisplayName = "Get Ability Component Data"))
	static void GetAbilityComponentData(const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_Ability& OutData);

	/** Returns the equipment component data for the specified item definition. */
	UFUNCTION(BlueprintPure, Category = "Itemization Core|Item Definition|Components", meta = (DisplayName = "Get Equipment Component Data"))
	static void GetEquipmentComponentData(const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_Equipment& OutData);

	/** Returns the icon component data for the specified item definition. */
	UFUNCTION(BlueprintPure, Category = "Itemization Core|Item Definition|Components", meta = (DisplayName = "Get Icon Component Data"))
	static void GetIconComponentData(const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_Icon& OutData);

	/** Returns the inventory item limit by tag component data for the specified item definition. */
	UFUNCTION(BlueprintPure, Category = "Itemization Core|Item Definition|Components", meta = (DisplayName = "Get Inventory Item Limit By Tag Component Data"))
	static void GetInventoryItemLimitByTagComponentData(const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_InventoryItemLimitByTag& OutData);

	/** Returns the max stack size component data for the specified item definition. */
	UFUNCTION(BlueprintPure, Category = "Itemization Core|Item Definition|Components", meta = (DisplayName = "Get Max Stack Size Component Data"))
	static void GetMaxStackSizeComponentData(const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_MaxStackSize& OutData);

	/** Returns the owned gameplay tags component data for the specified item definition. */
	UFUNCTION(BlueprintPure, Category = "Itemization Core|Item Definition|Components", meta = (DisplayName = "Get Owned Gameplay Tags Component Data"))
	static void GetOwnedGameplayTagsComponentData(const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_OwnedGameplayTags& OutData);

	/** Returns the slot size component data for the specified item definition. */
	UFUNCTION(BlueprintPure, Category = "Itemization Core|Item Definition|Components", meta = (DisplayName = "Get Slot Size Component Data"))
	static void GetSlotSizeComponentData(const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_SlotSize& OutData);

	/** Returns the traits component data for the specified item definition. */
	UFUNCTION(BlueprintPure, Category = "Itemization Core|Item Definition|Components", meta = (DisplayName = "Get Traits Component Data"))
	static void GetTraitsComponentData(const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_Traits& OutData);
};
