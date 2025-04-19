// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/ComponentData/ItemComponentData_Traits.h"

#include "Items/ItemDefinition.h"
#include "UObject/AssetRegistryTagsContext.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "ItemComponentData_Traits"
#endif

FItemComponentData_Traits::FItemComponentData_Traits()
{
}

bool FItemComponentData_Traits::HasTrait(const UItemDefinition* ItemDefinition, const FGameplayTag& Trait)
{
	if (!IsValid(ItemDefinition))
	{
		return false;
	}

	if (const FItemComponentData_Traits* TraitsData = ItemDefinition->GetItemComponent<FItemComponentData_Traits>())
	{
		return TraitsData->Traits.HasTag(Trait);
	}

	return false;
}


#if WITH_EDITOR
EDataValidationResult FItemComponentData_Traits::IsDataValid(FDataValidationContext& Context) const
{
	if (Traits.IsEmpty())
	{
		Context.AddWarning(LOCTEXT(
			"NoTraits",
			"No traits defined for this item component data. This component won't do anything"));

		return EDataValidationResult::Valid;
	}

	return Super::IsDataValid(Context);
}

#undef LOCTEXT_NAMESPACE
#endif