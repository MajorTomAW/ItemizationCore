// Copyright © 2025 MajorT. All Rights Reserved.


#include "ItemAssetTypeIdCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "ItemizationCoreEditor.h"
#include "ItemizationCoreSettings.h"
#include "Items/ItemDefinition.h"

#define LOCTEXT_NAMESPACE "ItemizationCoreEditor"

TSharedRef<IPropertyTypeCustomization> FItemAssetTypeIdCustomization::MakeInstance()
{
	return MakeShareable(new FItemAssetTypeIdCustomization);
}

void FItemAssetTypeIdCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	using FThis = FItemAssetTypeIdCustomization;
	AssetTypeHandle = PropertyHandle;
	AssetTypeIdHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemAssetTypeId, AssetTypeId));
	
	HeaderRow
	.NameContent()
	.VAlign(VAlign_Center)
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.VAlign(VAlign_Center)
	[
		SAssignNew(ComboButton, SComboButton)
		.ContentPadding(0.f)
		.IsEnabled(this, &FThis::IsAssetTypeChangeable)
		.OnGetMenuContent(this, &FThis::GenerateAssetTypePicker)
		.ButtonContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(this, &FThis::GetDisplayValueString)
			]
		]
	];
}

void FItemAssetTypeIdCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

bool FItemAssetTypeIdCustomization::IsAssetTypeChangeable() const
{
	// If we only have one item type, we don't need to show the combo-box.
	if (const UItemizationCoreSettings* Settings = UItemizationCoreSettings::Get())
	{
		return Settings->ItemTypes.Num() > 1;
	}

	return false;
}

TSharedRef<SWidget> FItemAssetTypeIdCustomization::GenerateAssetTypePicker()
{
	FOnItemAssetTypePicked OnTypeIdPicked(FOnItemAssetTypePicked::CreateSP
		(this, &FItemAssetTypeIdCustomization::OnAssetTypePicked, AssetTypeIdHandle ));
	
	return SNew(SBox)
		.WidthOverride(200.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(480.f)
			[
				IItemizationCoreEditorModule::Get().CreateItemAssetTypePicker(OnTypeIdPicked)
			]
		];
}

FText FItemAssetTypeIdCustomization::GetDisplayValueString() const
{
	if (AssetTypeIdHandle.IsValid())
	{
		FText DisplayText;
		AssetTypeIdHandle->GetValueAsDisplayText(DisplayText);
		return DisplayText;
	}

	return LOCTEXT("InvalidAssetType", "None");
}

void FItemAssetTypeIdCustomization::OnAssetTypePicked(const FName& PickedType, TSharedPtr<IPropertyHandle> NameProperty)
{
	if (NameProperty && NameProperty->IsValidHandle())
	{
		FScopedTransaction Transaction(LOCTEXT("OnAssetTypePicked", "Set Asset Type"));
		NameProperty->NotifyPreChange();
		NameProperty->SetValue(PickedType);
		NameProperty->NotifyPostChange(EPropertyChangeType::ValueSet);
		NameProperty->NotifyFinishedChangingProperties();
	}

	ComboButton->SetIsOpen(false);
}

#undef LOCTEXT_NAMESPACE
