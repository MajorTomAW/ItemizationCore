// Author: Tom Werner (MajorT), 2025


#include "Items/Data/ItemComponentData_OwnedGameplayTags.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "ItemComponentData_OwnedGameplayTags"
#endif


#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemComponentData_OwnedGameplayTags)

#if WITH_EDITOR
FText FItemComponentData_OwnedGameplayTags::GetDescription() const
{
	if (Tags.IsEmpty())
	{
		return FText::FromString(TEXT("None"));
	}

	FStringBuilderBase StringBuilder;
	StringBuilder.Append(TEXT("Tags:"));

	for (const auto& Tag : Tags)
	{
		StringBuilder.Append("\n\t" + Tag.GetTagName().ToString());
	}

	return FText::FromString(StringBuilder.ToString());
}

EDataValidationResult FItemComponentData_OwnedGameplayTags::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = FItemComponentData::IsDataValid(Context);

	if (Tags.IsEmpty())
	{
		Context.AddWarning(LOCTEXT("EmptyGameplayTagContainer", "OwnedGameplayTags is empty and has no function. Keeping it is just waste of performance."));
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
#endif
