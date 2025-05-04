// Author: Tom Werner (MajorT), 2025

#include "InventoryConfig.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "InventoryConfig"

EDataValidationResult UInventoryConfig::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Validate the inventory tag
	if (!InventoryTag.IsValid())
	{
		Context.AddWarning(LOCTEXT("InvalidInventoryTagWarning",
			"has an invalid inventory tag. You will have trouble identifying this inventory in the future."));
	}

	// Validate the inventory group configs
	for (const auto& Config : InventoryGroupConfigs)
	{
		if (!Config.GroupType.IsValid())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("InvalidGroupTypeError",
				"has an invalid group type '{1}'."),
				FText::AsCultureInvariant(Config.GroupType.ToString())));
		}

		if ((Config.NumItemColumns && Config.NumItemRows) == false)
		{
			// It doesn't make sense to have a group with no slots
			Result = EDataValidationResult::Invalid;
			
			Context.AddError(FText::Format(LOCTEXT("InvalidGroupSizeError",
				"has an invalid group size Rows: {1} Columns: {2}."),
				FText::AsNumber(Config.NumItemRows),
				FText::AsNumber(Config.NumItemColumns)));
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
#endif