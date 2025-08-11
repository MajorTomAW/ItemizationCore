// Author: Tom Werner (MajorT), 2025


#include "ItemDefinitionAssetFactory.h"

#include "ClassViewerModule.h"
#include "ItemizationEditorHelpers.h"
#include "Items/ItemDefinitionBase.h"
#include "Kismet2/SClassPickerDialog.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAssetFactory"

UItemDefinitionAssetFactory::UItemDefinitionAssetFactory()
{
	SupportedClass = UItemDefinitionBase::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

bool UItemDefinitionAssetFactory::ConfigureProperties()
{
	using namespace UE::ItemizationCore::Editor;

	if (!DoesClassHaveSubtypes(SupportedClass))
	{
		return true; // No subtypes, no need to configure
	}

	// nullptr the selected class to allow the user to choose a subtype
	SelectedItemClass = nullptr;

	// Create the class viewer filter
	TSharedPtr<FItemizationClassViewerFilter> Filter = MakeShareable(new FItemizationClassViewerFilter(SupportedClass));
	FClassViewerInitializationOptions Options;
	{
		Options.Mode = EClassViewerMode::ClassPicker;
		Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;
		Options.bShowNoneOption = false;
		Options.ClassFilters.Add(Filter.ToSharedRef());
		Options.ExtraPickerCommonClasses = CommonItemClasses;
	}

	// Pick the class using the class picker dialog
	const FText TitleText = LOCTEXT("CreateItemDefinitionOptions", "Pick Class for an Item Definition");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, SupportedClass);

	if (bPressedOk)
	{
		SelectedItemClass = ChosenClass;
	}

	return bPressedOk;
}

FText UItemDefinitionAssetFactory::GetDisplayName() const
{
	return LOCTEXT("ItemDefinitionAssetDisplayName", "Item Definition");
}

FString UItemDefinitionAssetFactory::GetDefaultNewAssetName() const
{
	return FString(TEXT("New")) + SupportedClass->GetName();
}

UObject* UItemDefinitionAssetFactory::FactoryCreateNew(
	UClass* InClass,
	UObject* InParent,
	FName InName,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn,
	FName CallingContext)
{
	if (SelectedItemClass != nullptr)
	{
		return NewObject<UItemDefinitionBase>(InParent, SelectedItemClass, InName, Flags | RF_Transactional, Context);
	}
	
	check(InClass->IsChildOf(UItemDefinitionBase::StaticClass()));
	return NewObject<UItemDefinitionBase>(InParent, InClass, InName, Flags | RF_Transactional, Context);
}

#undef LOCTEXT_NAMESPACE