// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Equipment/EquippableInventorySlotBinding.h"

#include "InventoryProperties.generated.h"

class UInventoryComponent;
class UActorComponent;
class UObject;
class FString;
class FText;

/** Base properties struct for an inventory type. */
USTRUCT(BlueprintType)
struct FInventoryPropertiesBase
{
	GENERATED_BODY()

	FInventoryPropertiesBase() = default;

public:
	/** The name or identifier of this inventory type. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Properties)
	FString InventoryName;

	/** The display name of this inventory to show in the HUD. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Properties)
	FText InventoryDisplayName;

	/** Description of this inventory to show in the HUD. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Properties)
	FText InventoryDescription;

	/** Override the hard limit for the number of items in this inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	int32 OverrideNumSlots = INDEX_NONE;

	/** Item requirement query that specifies whether an item can be placed in this inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	FGameplayTagQuery ItemRequirementQuery;
};

USTRUCT(BlueprintType)
struct FInventoryProperties : public FInventoryPropertiesBase
{
	GENERATED_BODY()

public:
	/** The inventory component that should be used for this inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Properties)
	TSoftClassPtr<UInventoryComponent> InventoryComponent;
};

USTRUCT(BlueprintType)
struct FEquippableInventoryProperties : public FInventoryPropertiesBase
{
	GENERATED_BODY()

public:
	/** The equippable inventory component that should be used for this inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Properties)
	TSoftClassPtr<UActorComponent> InventoryComponent;
	
	/** List of slot bindings for this inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Properties)
	TArray<FEquippableInventorySlotBinding> SlotBindings;
};