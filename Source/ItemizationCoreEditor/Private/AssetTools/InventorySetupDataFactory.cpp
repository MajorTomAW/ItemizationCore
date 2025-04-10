// Copyright © 2025 MajorT. All Rights Reserved.


#include "InventorySetupDataFactory.h"

#include "ClassViewerModule.h"
#include "InventorySetupDataBase.h"
#include "ItemizationEditorAssetFilter.h"
#include "Kismet2/SClassPickerDialog.h"

#define LOCTEXT_NAMESPACE "ItemizationCoreEditor"

UInventorySetupDataFactory::UInventorySetupDataFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UInventorySetupDataBase::StaticClass();
	SelectedClass = nullptr;
}

bool UInventorySetupDataFactory::ConfigureProperties()
{
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(SupportedClass, DerivedClasses, false);

	if (DerivedClasses.Num() <= 0)
	{
		return true;
	}

	// Nullptr the selected class so we can check for selection
	SelectedClass = nullptr;

	TSharedPtr<FItemizationEditorAssetFilter> Filter =
		MakeShared<FItemizationEditorAssetFilter>(SupportedClass);
	FClassViewerInitializationOptions Options;
	{
		Options.Mode = EClassViewerMode::ClassPicker;
		Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;
		Options.DisplayMode = EClassViewerDisplayMode::TreeView;
		Options.bShowDefaultClasses = true;
		Options.bShowNoneOption = false;
		Options.ClassFilters.Add(Filter.ToSharedRef());
		Options.ExtraPickerCommonClasses.Add(UInventorySetupDataBase_Default::StaticClass());
	}

	const FText TitleText = LOCTEXT("SelectInventorySetupDataClass", "Select Inventory Setup Data Class");
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, SelectedClass, SupportedClass);

	if (bPressedOk == false)
	{
		SelectedClass = nullptr;
	}

	return bPressedOk;
}

UObject* UInventorySetupDataFactory::FactoryCreateNew(
	UClass* InClass, UObject* InParent, FName InName,
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	if (SelectedClass != nullptr)
	{
		return NewObject<UInventorySetupDataBase>(InParent, SelectedClass, InName, Flags | RF_Transactional, Context);
	}

	check(InClass->IsChildOf(SupportedClass));
	return NewObject<UInventorySetupDataBase>(InParent, InClass, InName, Flags | RF_Transactional, Context);
}

FText UInventorySetupDataFactory::GetToolTip() const
{
	return Super::GetToolTip();
}

FText UInventorySetupDataFactory::GetDisplayName() const
{
	return LOCTEXT("InventorySetupDataDisplayName", "Inventory Setup Data");
}

#undef LOCTEXT_NAMESPACE
