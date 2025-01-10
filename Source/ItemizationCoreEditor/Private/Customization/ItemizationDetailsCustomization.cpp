// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationDetailsCustomization.h"

#include "DetailLayoutBuilder.h"
#include "Toolkits/ItemizationEditorApplication.h"


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

	TSharedPtr<FItemizationEditorApplication> App = WeakApp.Pin();
	const FName& Mode = App->GetCurrentMode();
	const UClass* ItemClass = App->GetItemDefinition()->GetClass();

	const bool bModeExclusive = ItemClass->HasMetaData("ModesExclusive");

	FIsPropertyVisible IsVisible = FIsPropertyVisible::CreateLambda([Mode, bModeExclusive, ItemClass](const FPropertyAndParent& Prop)->bool
	{
		if (Prop.Property.HasMetaData("AllowedItemModes"))
		{
			const FString AllowedModes = Prop.Property.GetMetaData("AllowedItemModes");
			if (!AllowedModes.Contains(Mode.ToString()))
			{
				return false;
			}
		}
		else if (bModeExclusive)
		{
			const FString ExclusiveModes = ItemClass->GetMetaData("ModesExclusive");
			if (ExclusiveModes.Contains(Mode.ToString()))
			{
				return false;
			}
		}
		
		return true;
	});

	DetailBuilder.GetDetailsView()->SetIsPropertyVisibleDelegate(IsVisible);
}
