// Author: Tom Werner (MajorT), 2025


#include "Items/Data/ItemComponentData_Traits.h"

#include "ItemizationCoreTags.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "ItemComponentData_OwnedGameplayTags"
#endif

FItemComponentData_Traits::FItemComponentData_Traits()
{
	Traits.AddTag(Itemization::Tags::TAG_ItemTrait_AutoCombineStacks);
	Traits.AddTag(Itemization::Tags::TAG_ItemTrait_InventorySizeLimited);
}

bool FItemComponentData_Traits::HasTrait(const FGameplayTag& TraitToCheck) const
{
	return Traits.HasTagExact(TraitToCheck);
}

#if WITH_EDITOR
EDataValidationResult FItemComponentData_Traits::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = FItemComponentData::IsDataValid(Context);

	if (Traits.IsEmpty())
	{
		Context.AddWarning(LOCTEXT("EmptyGameplayTraits", "Traits is empty and has no function. Keeping it is just waste of performance."));
	}

	return Result;
}

FText FItemComponentData_Traits::GetDescription() const
{
	if (Traits.IsEmpty())
	{
		return FText::FromString(TEXT("None"));
	}

	FStringBuilderBase StringBuilder;
	StringBuilder.Append(TEXT("Traits:"));

	for (const auto& Tag : Traits)
	{
		StringBuilder.Append("\n\t" + Tag.GetTagName().ToString());
	}

	return FText::FromString(StringBuilder.ToString());
}

#undef LOCTEXT_NAMESPACE
#endif
