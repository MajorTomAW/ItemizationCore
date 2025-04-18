// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/ComponentData/ItemComponentData_Traits.h"

#include "UObject/AssetRegistryTagsContext.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "ItemComponentData_Traits"
#endif

FItemComponentData_Traits::FItemComponentData_Traits()
{
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