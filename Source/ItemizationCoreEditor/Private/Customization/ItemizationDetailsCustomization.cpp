// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationDetailsCustomization.h"

#include "DetailLayoutBuilder.h"


FItemizationDetailsCustomization::FItemizationDetailsCustomization()
{
}

TSharedRef<IDetailCustomization> FItemizationDetailsCustomization::MakeInstance(
	const FItemizationDetailsViewArgs& InArgs, const TSharedPtr<FItemizationEditorApplication>& InApp)
{
	TSharedRef Instance = MakeShared<FItemizationDetailsCustomization>();
	Instance->Args = InArgs;
	Instance->WeakApp = InApp;
	return Instance;
}

void FItemizationDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	for (const FName& Category : Args.CategoryBlackList)
	{
		DetailBuilder.HideCategory(Category);
	}
	for (const FName& Property : Args.PropertyBlackList)
	{
		DetailBuilder.HideProperty(Property);
	}
	
	if (Args.CategoryWhiteList.Num() > 0)
	{
		TArray<FName> Categories;
		DetailBuilder.GetCategoryNames(Categories);

		for (const FName& Category : Categories)
		{
			if (!Args.CategoryWhiteList.Contains(Category))
			{
				DetailBuilder.HideCategory(Category);
			}
		}
	}
}
