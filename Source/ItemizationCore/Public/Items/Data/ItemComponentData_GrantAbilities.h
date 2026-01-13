// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "GameplayAbilitySpecHandle.h"
#include "ItemComponentData.h"
#include "Enums/EItemStateFilter.h"

#include "ItemComponentData_GrantAbilities.generated.h"

class IInventoryAbilityItemInstanceInterface;
class UGameplayEffect;
class UGameplayAbility;

/** Single entry for an ability to be granted. */
USTRUCT(BlueprintType)
struct FItemAbilityGrant
{
	GENERATED_BODY()

public:
	/** When this ability grant should be granted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityGrant)
	EItemStateFilter ActiveState = EItemStateFilter::Equipped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityGrant)
	TArray<TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityGrant)
	TArray<TSubclassOf<UGameplayEffect>> Effects;
};

/** Item data for granting abilities to the owning actor. */
USTRUCT(DisplayName="Grant Abilities Item Data")
struct FItemComponentData_GrantAbilities : public FItemComponentData
{
	GENERATED_BODY()

public:
	/** List of ability grants to be given. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityGrant)
	TArray<FItemAbilityGrant> AbilityGrants;

protected:
	//~ Begin FItemComponentData Interface
#if WITH_EDITOR
	virtual FText GetDescription() const override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	virtual void OnItemGiven(FInventoryItemEntry& ItemEntry, const FInventoryHandle& InventoryHandle) const override;
	virtual void OnItemRemoved(FInventoryItemEntry& ItemEntry, const FInventoryHandle& InventoryHandle) const override;
	//~ End FItemComponentData Interface

private:
	IInventoryAbilityItemInstanceInterface* GetAbilityItemInstance(const FInventoryItemEntry& ItemEntry) const;
};
