// Author: Tom Werner (MajorT), 2025

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "InstancedStructDetails.h"
#include "IPropertyUtilities.h"
#include "ItemComponentDataCustomization.h"
#include "Items/Data/ItemComponentData.h"

#include "Slate/Pickers/SItemComponentDataPicker.h"
#include "Styles/ItemizationEditorStyle.h"

TSharedRef<IPropertyTypeCustomization> FItemComponentDataCustomization::MakeInstance()
{
	return MakeShared<FItemComponentDataCustomization>();
}

FItemComponentDataCustomization::~FItemComponentDataCustomization()
{
	if (OnObjectsReInstancedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectsReinstanced.Remove(OnObjectsReInstancedHandle);
	}
}

void FItemComponentDataCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	InstanceProperty = PropertyHandle;
	PropUtils = CustomizationUtils.GetPropertyUtilities();
	StructProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemComponentDataInstance, Component));
	check(StructProperty);

	ArrayProperty = InstanceProperty->GetParentHandle()->AsArray();

	OnObjectsReInstancedHandle = FCoreUObjectDelegates::OnObjectsReinstanced.AddSP(this, &ThisClass::OnObjectsReInstanced);

	HeaderRow
		.NameContent()
		[
			SNew(SHorizontalBox)
			.ToolTipText(this, &ThisClass::GetTooltipText)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0.f, 0.f, 4.f, 0.f))
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2d(16.f))
				.Image(FItemizationEditorStyle::Get()->GetBrush("Icons.Components"))
			]

			// Label
			+ SHorizontalBox::Slot()
			.Padding(0.f)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &ThisClass::GetDisplayValueText)
			]
		]
		.ValueContent()
		.MinDesiredWidth(250.f)
		.VAlign(VAlign_Center)
		[
			SAssignNew(StructPicker, SItemComponentDataPicker, StructProperty, InstanceProperty, PropUtils)
		]
		.IsEnabled(StructProperty->IsEditable());
}

void FItemComponentDataCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedRef<FInstancedStructDataDetails> Details = MakeShared<FInstancedStructDataDetails>(StructProperty);
	ChildBuilder.AddCustomBuilder(Details);
}

void FItemComponentDataCustomization::OnObjectsReInstanced(
	const FReplacementObjectMap& ObjectMap)
{
	// Force update the details when BP is compiled, since we may cached hold references to the old object or class.
	if (!ObjectMap.IsEmpty() && PropUtils.IsValid())
	{
		PropUtils->RequestRefresh();
	}
}

FText FItemComponentDataCustomization::GetDisplayValueText() const
{
	const UScriptStruct* CommonStruct = nullptr;
	const FPropertyAccess::Result Result = GetCommonScriptStruct(StructProperty, CommonStruct);

	if (Result == FPropertyAccess::Success)
	{
		if (CommonStruct)
		{
			return CommonStruct->GetDisplayNameText();
		}
		return NSLOCTEXT("ItemizationEditor", "NullScriptStruct", "None");
	}
	if (Result == FPropertyAccess::MultipleValues)
	{
		return NSLOCTEXT("ItemizationEditor", "MultipleValues", "Multiple Values");
	}

	return FText::GetEmpty();
}

FText FItemComponentDataCustomization::GetTooltipText() const
{
	const UScriptStruct* CommonStruct = nullptr;
	const FPropertyAccess::Result Result = GetCommonScriptStruct(StructProperty, CommonStruct);
	if (CommonStruct && Result == FPropertyAccess::Success)
	{
		return CommonStruct->GetToolTipText();
	}

	return GetDisplayValueText();
}
