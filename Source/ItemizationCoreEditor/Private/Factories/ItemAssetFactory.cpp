// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemAssetFactory.h"

#include "ClassViewerModule.h"
#include "ItemDefinition.h"
#include "ItemizationCoreEditor.h"
#include "ItemizationCoreEditorHelpers.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Settings/ItemizationCoreEditorSettings.h"

#define LOCTEXT_NAMESPACE "ItemAssetFactory"

using namespace UE::ItemizationCore::Editor;

UItemAssetFactory::UItemAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UItemDefinition::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

bool UItemAssetFactory::ConfigureProperties()
{
	if (!DoesClassHaveSubtypes(UItemDefinition::StaticClass()))
	{
		return true;
	}

	// nullptr the ItemDefinitionClass so we can check for selection
	ItemDefinitionClass = nullptr;
	
	TSharedPtr<FItemizationClassViewerFilter> Filter = MakeShareable(new FItemizationClassViewerFilter(UItemDefinition::StaticClass()));
	FClassViewerInitializationOptions Options;
	{
		Options.Mode = EClassViewerMode::ClassPicker;
		Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;
		Options.bShowNoneOption = false;
		Options.ClassFilters.Add(Filter.ToSharedRef());
		Options.ExtraPickerCommonClasses = UItemizationCoreEditorSettings::Get()->CommonItemClasses;
	}

	const FText TitleText = LOCTEXT("CreateItemDefinitionOptions", "Pick Class for an Item Definition");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UItemDefinition::StaticClass());

	if (bPressedOk)
	{
		ItemDefinitionClass = ChosenClass;
	}

	return bPressedOk;
}

FText UItemAssetFactory::GetDisplayName() const
{
	return LOCTEXT("ItemAssetFactoryDisplayName", "Item Definition");
}

FString UItemAssetFactory::GetDefaultNewAssetName() const
{
	return FString(TEXT("New")) + UItemDefinition::StaticClass()->GetName();
}

UObject* UItemAssetFactory::FactoryCreateNew(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	if (ItemDefinitionClass != nullptr)
	{
		return NewObject<UItemDefinition>(InParent, ItemDefinitionClass, InName, Flags | RF_Transactional, Context);
	}
	
	check(InClass->IsChildOf(UItemDefinition::StaticClass()));
	return NewObject<UItemDefinition>(InParent, InClass, InName, Flags | RF_Transactional, Context);
}

#undef LOCTEXT_NAMESPACE