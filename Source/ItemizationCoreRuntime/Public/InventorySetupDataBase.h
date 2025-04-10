// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"

#include "InventorySetupDataBase.generated.h"

class UObject;
class UInputAction;
class FString;
class FText;

/**
 * Data class that is used for setting up an inventory system.
 */
UCLASS(BlueprintType, Blueprintable, MinimalAPI, Const, Abstract)
class UInventorySetupDataBase : public UObject
{
	GENERATED_BODY()

public:
	UInventorySetupDataBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

/** 
 * Base struct for inventory properties.
 * This is used to define custom properties for the inventory system.
 */
USTRUCT(BlueprintType)
struct FInventoryPropertiesBase
{
	GENERATED_BODY()

public:
	FInventoryPropertiesBase();

	/** The name or identifier of the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FString InventoryName;

	/** The friendly name of the inventory to show in the HUD. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FText InventoryDisplayName;

	/** The description of the inventory to show in the HUD. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory, meta = (MultiLine = true))
	FText InventoryDescription;

	/** The total number of slots in the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	int64 TotalSlotsOverride;
};

/**
 * The default inventory setup properties struct.
 */
USTRUCT(BlueprintType)
struct FInventoryProperties : public FInventoryPropertiesBase
{
	GENERATED_BODY()
	
public:
	/** The class of the inventory to spawn. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TSoftClassPtr<AActor> InventoryClass;
};

/**
 * Struct holding data about the binding between an inventory slot and an input action.
 */
USTRUCT(BlueprintType, DisplayName="Equippable Slot Binding")
struct FEquippableInventorySlotBindingDefinition
{
	GENERATED_BODY()

	FEquippableInventorySlotBindingDefinition()
		: Slot(0)
	{
	}
	
public:
	/** The slot id of this binding. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	uint32 Slot;

	/** The input action to bind to this slot. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TSoftObjectPtr<UInputAction> InputAction;
};

/**
 * Struct holding data about an equippable inventory.
 */
USTRUCT(BlueprintType)
struct FEquippableInventoryProperties : public FInventoryPropertiesBase
{
	GENERATED_BODY()
	
public:
	/** The class of the inventory to spawn. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TSoftClassPtr<AActor> InventoryClass;

	/** The slot bindings to create for this inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory, NoClear)
	TArray<TInstancedStruct<FEquippableInventorySlotBindingDefinition>> SlotBindings;
};

/**
 * Struct holding data about an item that should be given on startup or when the inventory gets created the first time.
 */
USTRUCT(BlueprintType)
struct FInventoryStartingItem
{
	GENERATED_BODY()
	
	FInventoryStartingItem()
		: ItemDefinition(nullptr)
		, EntityClassFilter(nullptr)
		, Amount(1)
	{
	}

public:
	/** The item definition to add to the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TSoftObjectPtr<UObject> ItemDefinition;

	/** The entity class filter that will be used to determine if this item can be added to the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TSoftClassPtr<UObject> EntityClassFilter;

	/** The amount of items to add to the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	uint32 Amount;
};

/**
 * The default setup data class for an inventory system.
 */
UCLASS(MinimalAPI)
class UInventorySetupDataBase_Default : public UInventorySetupDataBase
{
	GENERATED_BODY()

public:
	UInventorySetupDataBase_Default(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	/** The default properties for the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory, NoClear)
	TArray<TInstancedStruct<FInventoryProperties>> InventoryList;

	/** The default properties for an equippable inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory, NoClear)
	TArray<TInstancedStruct<FEquippableInventoryProperties>> EquippableInventoryList;

	/** The list of starting items to add to the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory, NoClear)
	TArray<TInstancedStruct<FInventoryStartingItem>> StartingItemList;
};