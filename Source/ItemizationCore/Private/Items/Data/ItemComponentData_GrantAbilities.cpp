// Author: Tom Werner (dc: majort), 2026 January


#include "Items/Data/ItemComponentData_GrantAbilities.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "InventoryHandle.h"
#include "ItemizationCoreLogChannels.h"
#include "Inventory/InventoryBase.h"
#include "Items/IInventoryAbilityItemInstanceInterface.h"

#if WITH_EDITOR
FText FItemComponentData_GrantAbilities::GetDescription() const
{
	return FItemComponentData::GetDescription();
}

EDataValidationResult FItemComponentData_GrantAbilities::IsDataValid(FDataValidationContext& Context) const
{
	return FItemComponentData::IsDataValid(Context);

	//@TODO: Make sure item has an instance and that instance inherits the IInventoryAbilityItemInstanceInterface
}
#endif

void FItemComponentData_GrantAbilities::OnItemGiven(
	FInventoryItemEntry& ItemEntry,
	const FInventoryHandle& InventoryHandle) const
{
	IInventoryAbilityItemInstanceInterface* AbilityItem = GetAbilityItemInstance(ItemEntry);
	if (AbilityItem == nullptr)
	{
		ITEMIZATION_WARN("No Item Instance most likely")
		return;
	}

	// Give Abilities & Effects
	TArray<FGameplayAbilitySpecHandle> GrantedAbilities;
	TArray<FActiveGameplayEffectHandle> GrantedEffects;
	for (const FItemAbilityGrant& Grant : AbilityGrants)
	{
		for (const TSubclassOf<UGameplayAbility>& Class : Grant.Abilities)
		{
			AbilityItem->TryGiveAbility(Class, 1, INDEX_NONE, GetFName());
		}

		for (const TSubclassOf<UGameplayEffect>& EffectClass : Grant.Effects)
		{
			AbilityItem->TryApplyGameplayEffect(EffectClass, 1.f, GetFName());
		}
	}
}

void FItemComponentData_GrantAbilities::OnItemRemoved(
	FInventoryItemEntry& ItemEntry,
	const FInventoryHandle& InventoryHandle) const
{
	/*AActor* InventoryOwner = InventoryHandle.GetInventory()->GetOwner();
	UAbilitySystemComponent* OwnerASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InventoryOwner);

	if (!IsValid(OwnerASC))
	{
		return;
	}

	IInventoryAbilityItemInstanceInterface* AbilityItem = GetAbilityItemInstance(ItemEntry);
	if (AbilityItem == nullptr)
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Granted : AbilityItem->Get)
	{
		OwnerASC->ClearAbility(Granted);
	}
	GrantedHandles.Empty();*/
}

IInventoryAbilityItemInstanceInterface* FItemComponentData_GrantAbilities::GetAbilityItemInstance(
	const FInventoryItemEntry& ItemEntry) const
{
	UObject* ItemInstance = nullptr;
	if (TScriptInterface InstanceInterface = ItemEntry.GetItemInstance())
	{
		ItemInstance = InstanceInterface.GetObject();
	}

	if (!IsValid(ItemInstance))
	{
		return nullptr;
	}

	return Cast<IInventoryAbilityItemInstanceInterface>(ItemInstance);
}
