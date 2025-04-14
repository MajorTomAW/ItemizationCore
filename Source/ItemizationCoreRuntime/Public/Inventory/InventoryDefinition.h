// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "InventoryDefinition.generated.h"

class UInputAction;
/** Class defining a single inventory in the game. */
UCLASS(BlueprintType, Const, MinimalAPI)
class UInventoryDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UInventoryDefinition();
	
	/**
	 * The name or identifier for inventories of this type.
	 * The player will most likely never see this.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FText Name;

	/**
	 * The description for inventories of this type.
	 * The player will most likely never see this.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FText Description;

	/** The number of slots this inventory type has. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	uint16 NumSlots = 100;

	/**
	 * The pickup priority of this inventory type.
	 * Inventories with a higher priority will get the picked-up items prior over other inventories.
	 * Bigger numbers mean higher priority.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	uint8 PickupPriority = 0;

	/** If set to false, will globally disable item stacking for this inventory type. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	bool bEnableItemStacking = true;

	/** Optional filter class that will be used to determine if an item can be put into this inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TObjectPtr<UObject> ItemRequirements = nullptr;

	/** List of optional gameplay tags to further define this inventory type. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FGameplayTagContainer GameplayTags;
};

/** Class defining an equippable inventory in the game. */
UCLASS(BlueprintType, Const, MinimalAPI)
class UEquippableInventoryDefinition : public UInventoryDefinition
{
	GENERATED_BODY()

public:
	UEquippableInventoryDefinition();

	/** Optional equipment restrictions that can disallow certain items from being equipped. */
	UPROPERTY(EditDefaultsOnly, Category = Equipment)
	TArray<uint8> EquipRestrictions;

	/** List of input actions that can be bound to equipment slots. */
	UPROPERTY(EditDefaultsOnly, Category = Equipment)
	TArray<TObjectPtr<UInputAction>> SlotBindings;

	/** If true, will show the slot bindings as an input key in the UI. */
	UPROPERTY(EditDefaultsOnly, Category = Equipment)
	bool bShowSlotBindings = true;
};
