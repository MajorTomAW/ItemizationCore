/*
// Author: Tom Werner (MajorT), 2025


#include "Items/Data/ItemComponentData_PlayerGrantedTags.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "ItemComponentData_OwnedGameplayTags"
#endif

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "InventoryHandle.h"
#include "Inventory/InventoryBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemComponentData_PlayerGrantedTags)

void FItemComponentData_PlayerGrantedTags::OnItemGiven(
	FInventoryItemEntry& ItemEntry,
	const FInventoryHandle& InventoryHandle) const
{
	if (AInventoryBase* OwningInventory = InventoryHandle.GetInventory())
	{
		if (UAbilitySystemComponent* OwnerACS = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningInventory->GetOwner()))
		{
			OwnerACS->AddLooseGameplayTags(TagsToGrant);

			if (bShouldReplicate)
			{
				OwnerACS->AddReplicatedLooseGameplayTags(TagsToGrant);
			}
		}
	}
}

void FItemComponentData_PlayerGrantedTags::OnItemRemoved(
	FInventoryItemEntry& ItemEntry,
	const FInventoryHandle& InventoryHandle) const
{
	if (AInventoryBase* OwningInventory = InventoryHandle.GetInventory())
	{
		if (UAbilitySystemComponent* OwnerACS = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningInventory->GetOwner()))
		{
			OwnerACS->RemoveLooseGameplayTags(TagsToGrant);

			if (bShouldReplicate)
			{
				OwnerACS->RemoveReplicatedLooseGameplayTags(TagsToGrant);
			}
		}
	}
}

#if WITH_EDITOR
EDataValidationResult FItemComponentData_PlayerGrantedTags::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = FItemComponentData::IsDataValid(Context);

	if (TagsToGrant.IsEmpty())
	{
		Context.AddWarning(LOCTEXT("EmptyGameplayTags", "TagsToGrant is empty and has no function. Keeping it is just waste of performance."));
	}

	return Result;
}

FText FItemComponentData_PlayerGrantedTags::GetDescription() const
{
	if (TagsToGrant.IsEmpty())
	{
		return FText::FromString(TEXT("None"));
	}

	FStringBuilderBase StringBuilder;
	StringBuilder.Append(TEXT("Tags to Grant:"));

	for (const auto& Tag : TagsToGrant)
	{
		StringBuilder.Append("\n\t" + Tag.GetTagName().ToString());
	}

	return FText::FromString(StringBuilder.ToString());
}
#undef LOCTEXT_NAMESPACE
#endif
*/
