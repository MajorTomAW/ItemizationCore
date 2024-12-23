// Copyright Epic Games, Inc. All Rights Reserved.


#include "SItemizationEditorAssetType.h"

#include "EditorClassUtils.h"
#include "ItemDefinition.h"
#include "SlateOptMacros.h"
#include "Styling/SlateIconFinder.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemizationEditorAssetType::Construct(const FArguments& InArgs)
{
	ItemDefinition = InArgs._ItemDefinition;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(FMargin(12.f, 0.f))
			[
				SNew(SBorder)
				.Padding(FMargin(6.f, 2.f))
				.BorderImage(FAppStyle::GetBrush("ToolBar.Button"))
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FStyleColors::Recessed)
				.Content()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(12.f, 0.f)
					[
						SNew(SHorizontalBox)

						// Class Icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Left)
						.Padding(0.f, 0.f, 5.f, 0.f)
						[
							SNew(SImage)
							.Image(this, &SItemizationEditorAssetType::HandleGetAssetIcon)
						]

						// Class Name Text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							FEditorClassUtils::GetSourceLink(GetAssetClass())
						]
					]
				]
			]
		]
	];
}

FText SItemizationEditorAssetType::HandleGetAssetTypeText() const
{
	const UItemDefinition* ItemDef = ItemDefinition.Get();
	if (ItemDef == nullptr)
	{
		return FText::FromString(TEXT("None"));
	}

	return ItemDef->GetClass()->GetDisplayNameText();
}

const FSlateBrush* SItemizationEditorAssetType::HandleGetAssetIcon() const
{
	const UItemDefinition* ItemDef = ItemDefinition.Get();
	if (ItemDef == nullptr)
	{
		return FSlateIconFinder::FindIconBrushForClass(UItemDefinition::StaticClass());
	}

	return FSlateIconFinder::FindIconBrushForClass(ItemDef->GetClass());
}

UClass* SItemizationEditorAssetType::GetAssetClass() const
{
	const UItemDefinition* ItemDef = ItemDefinition.Get();
	if (ItemDef == nullptr)
	{
		return UItemDefinition::StaticClass();
	}

	return ItemDef->GetClass();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
