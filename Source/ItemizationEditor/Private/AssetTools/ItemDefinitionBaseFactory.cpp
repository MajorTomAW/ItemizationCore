// Author: Tom Werner (MajorT), 2025 November


#include "ItemDefinitionBaseFactory.h"

#include "ItemizationEditorHelpers.h"
#include "Items/ItemDefinitionBase.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Toolkits/IItemDefinitionApp.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemDefinitionBaseFactory)

#define LOCTEXT_NAMESPACE "ItemDefinitionBaseFactory"

UItemDefinitionBaseFactory::UItemDefinitionBaseFactory()
{
	SupportedClass = UItemDefinitionBase::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

bool UItemDefinitionBaseFactory::ConfigureProperties()
{
	using namespace UE::ItemizationEditor;
	if (!DoesClassHaveSubtypes(SupportedClass))
	{
		return true; // No subtypes, no need to spawn an item picker
	}

	// nullptr the selected class to allow the user to choose a new one
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

FText UItemDefinitionBaseFactory::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "Item Definition");
}

FString UItemDefinitionBaseFactory::GetDefaultNewAssetName() const
{
	return FString(TEXT("New")) + SupportedClass->GetName();
}

UObject* UItemDefinitionBaseFactory::FactoryCreateNew(
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
