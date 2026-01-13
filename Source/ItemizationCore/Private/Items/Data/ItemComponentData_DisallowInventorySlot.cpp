// Author: Tom Werner (MajorT), 2025


#include "Items/Data/ItemComponentData_DisallowInventorySlot.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "ItemComponentData_OwnedGameplayTags"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemComponentData_DisallowInventorySlot)

FItemComponentData_DisallowInventorySlot::FItemComponentData_DisallowInventorySlot()
{
}

#if WITH_EDITOR
EDataValidationResult FItemComponentData_DisallowInventorySlot::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = FItemComponentData::IsDataValid(Context);

	if (DisallowedSlotTags.IsEmpty())
	{
		Context.AddWarning(LOCTEXT("EmptyDisallowedSlots", "Disallowed Slots is empty and has no function. Keeping it is just waste of performance."));
	}

	return Result;
}

FText FItemComponentData_DisallowInventorySlot::GetDescription() const
{
	if (DisallowedSlotTags.IsEmpty())
	{
		return FText::FromString(TEXT("None"));
	}

	FStringBuilderBase StringBuilder;
	StringBuilder.Append(TEXT("Disallowed Slots:"));

	for (const auto& Tag : DisallowedSlotTags)
	{
		StringBuilder.Append("\n\t" + Tag.GetTagName().ToString());
	}

	return FText::FromString(StringBuilder.ToString());
}
#undef LOCTEXT_NAMESPACE
#endif
