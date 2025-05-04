// Author: Tom Werner (MajorT), 2025


#include "Items/Data/ItemComponentData_Traits.h"

#include "Items/ItemDefinitionBase.h"

FItemComponentData_Traits::FItemComponentData_Traits()
{
}

bool FItemComponentData_Traits::HasTrait(const UItemDefinitionBase* InItemDefinition, const FGameplayTag& TraitToCheck)
{
	if (const FItemComponentData_Traits* TraitsData = InItemDefinition->GetItemData<FItemComponentData_Traits>())
	{
		return TraitsData->Traits.HasTagExact(TraitToCheck);
	}

	return false;
}

#if WITH_EDITOR
EDataValidationResult FItemComponentData_Traits::IsDataValid(FDataValidationContext& Context) const
{
	return FItemComponentData::IsDataValid(Context);
}
#endif