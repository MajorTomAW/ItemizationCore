// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "Components/ItemComponentData.h"

#include "ItemComponentData_Ability.generated.h"

/**
 * Ability item data structure.
 */
USTRUCT(BlueprintType)
struct FAbilityItemComponent_Ability
{
	GENERATED_BODY()
	FAbilityItemComponent_Ability();

public:
	/** When should this ability be active (when this item is equipped, ...)? */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	EUserFacingItemState ActiveState;

	/** The ability itself. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (MetaClass = "/Script/GameplayAbilities.GameplayAbility"))
	FSoftClassPath Ability;

	/** The input gameplay tag associated with an input action used to trigger this ability. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (Categories = "Input.InputTag"))
	FGameplayTag InputTag;

	/** The default level of the ability. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FScalableFloat DefaultLevel;
};


/**
 * Item data for adding abilities to the item owner.
 */
USTRUCT(DisplayName = "Ability Item Data", meta = (SingletonComponent = true))
struct FItemComponentData_Ability : public FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData_Ability();
	
	/** List of abilities that will be added to the item owner. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<FAbilityItemComponent_Ability> AbilityList;

protected:
	//~ Begin FItemComponentData Interface
	virtual void OnItemInstanceCreated(const FInventoryItemEntryHandle& Handle, const FItemizationCoreInventoryData* InventoryData) const override;
	virtual void OnItemInstanceDestroyed(const FInventoryItemEntryHandle& Handle, const FItemizationCoreInventoryData* InventoryData) const override;
	virtual void OnItemStateChanged(const FInventoryItemEntryHandle& Handle, EUserFacingItemState NewState) const override;
	//~ End FItemComponentData Interface

private:
	/** Will be called after the asset manager has loaded all abilities. */
	void GrantAbilitiesDeferred();
};