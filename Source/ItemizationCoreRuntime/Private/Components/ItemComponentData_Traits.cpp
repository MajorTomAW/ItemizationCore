// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ItemComponentData_Traits.h"

#include "ItemizationCoreTags.h"
#include "ActorComponents/InventoryManager.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "ItemizationCore"

FItemComponentData_Traits::FItemComponentData_Traits()
{
	Traits.AddTag(ItemizationCoreTags::TAG_ItemTrait_Transient);
	Traits.AddTag(ItemizationCoreTags::TAG_ItemTrait_InventorySizeLimited);
}

bool FItemComponentData_Traits::CanCreateNewStack(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& Context) const
{
	if (Traits.IsEmpty())
	{
		return true;
	}

	check(Context.InventoryManager.IsValid());
	UInventoryManager* InventoryManager = Context.InventoryManager.Get();

	// Big if / else chain to check for various traits :(
	// But I guess it is fine because there will only be a few traits.
	if (Traits.HasTagExact(ItemizationCoreTags::TAG_ItemTrait_SingleStack))
	{
		TArray<FInventoryItemEntry*> ItemsOfType;
		InventoryManager->GetAllItemsOfType(&ItemsOfType, ItemEntry.Definition);

		if (ItemsOfType.Num() > 0)
		{
			return false;
		}
	}
	
	return true;
}

bool FItemComponentData_Traits::CanClearItem(const FInventoryItemEntry& ItemEntry) const
{
	if (Traits.IsEmpty())
	{
		return true;
	}

	return !Traits.HasTagExact(ItemizationCoreTags::TAG_ItemTrait_AllowEmptyFinalStack);
}

bool FItemComponentData_Traits::IncludeInClearAll() const
{
	if (Traits.IsEmpty())
	{
		return true;
	}

	return !Traits.HasTagExact(ItemizationCoreTags::TAG_ItemTrait_IgnoreRemoveAllInventoryItems);
}

#if WITH_EDITOR
EDataValidationResult FItemComponentData_Traits::IsDataValid(class FDataValidationContext& Context) const
{
	if (Traits.IsEmpty())
	{
		Context.AddWarning( LOCTEXT("NoTraits", "Traits Item Data: No traits defined. This component won't do anything.") );
		return EDataValidationResult::Valid;
	}
	
	return Super::IsDataValid(Context);
}
#endif

#undef LOCTEXT_NAMESPACE