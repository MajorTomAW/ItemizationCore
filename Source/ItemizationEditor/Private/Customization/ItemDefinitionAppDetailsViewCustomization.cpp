// Author: Tom Werner (MajorT), 2025 November


#include "ItemDefinitionAppDetailsViewCustomization.h"

#include "DetailLayoutBuilder.h"

UE::ItemizationEditor::FAssetDetailsViewCustomization::FAssetDetailsViewCustomization()
	: WeakApp(nullptr)
{
}

TSharedRef<IDetailCustomization> UE::ItemizationEditor::FAssetDetailsViewCustomization::MakeInstance(
	const FAssetDetailsViewArgs& InArgs, const TSharedPtr<FItemDefinitionApp>& InApp)
{
	TSharedRef Instance = MakeShared<FAssetDetailsViewCustomization>();
	Instance->Args = InArgs;
	Instance->WeakApp = InApp;
	return Instance;
}

void UE::ItemizationEditor::FAssetDetailsViewCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	for (const auto& Black : Args.CategoryBlacklist)
	{
		DetailBuilder.HideCategory(Black);
	}


	if (!Args.CategoryWhitelist.IsEmpty())
	{
		TArray<FName> Categories;
		DetailBuilder.GetCategoryNames(Categories);

		for (const auto& Category : Categories)
		{
			if (!Args.CategoryWhitelist.Contains(Category))
			{
				DetailBuilder.HideCategory(Category);
			}
		}
	}
}
