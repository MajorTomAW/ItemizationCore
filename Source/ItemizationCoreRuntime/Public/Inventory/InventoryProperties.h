// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/EquippableInventorySlotBinding.h"
#include "Slots/FInventorySlotProcessor.h"
#include "StructUtils/InstancedStruct.h"

#include "InventoryProperties.generated.h"

class FString;
class FText;
class AInventoryBase;
class AInventory;
class AEquippableInventory;
class ASlottableInventory;

/** 
 * Base struct for inventory properties.
 * This is used to define custom properties for the inventory system.
 */
USTRUCT(BlueprintType)
struct FInventoryPropertiesBase
{
	GENERATED_BODY()
	
	FInventoryPropertiesBase();
	virtual ~FInventoryPropertiesBase() = default;

public:
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
	FInventoryProperties();
	
public:
	/** The class of the inventory to spawn. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TSoftClassPtr<AInventory> InventoryClass;
};

/**
 * The default slottable inventory setup properties struct.
 */
USTRUCT(BlueprintType)
struct FSlottableInventoryProperties : public FInventoryPropertiesBase
{
	GENERATED_BODY()
	FSlottableInventoryProperties();

public:
	/** The class of the inventory to spawn. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TSoftClassPtr<ASlottableInventory> InventoryClass;

	/** List of slot processors that can be used to modify the slots in this inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory, NoClear, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FFInventorySlotProcessor>> SlotProcessors;
};

/**
 * Struct holding data about an equippable inventory.
 */
USTRUCT(BlueprintType)
struct FEquippableInventoryProperties : public FInventoryPropertiesBase
{
	GENERATED_BODY()
	FEquippableInventoryProperties();
	
public:
	/** The class of the inventory to spawn. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TSoftClassPtr<AEquippableInventory> InventoryClass;

	/** The slot bindings to create for this inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory, NoClear)
	TArray<TInstancedStruct<FEquippableInventorySlotBindingDefinition>> SlotBindings;
};