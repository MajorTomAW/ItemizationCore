// Author: Tom Werner (MajorT), 2025 November


#include "SItemDefinitionAppAssetType.h"

#include "EditorClassUtils.h"
#include "SlateOptMacros.h"
#include "Items/ItemDefinitionBase.h"
#include "Styling/SlateIconFinder.h"
#include "Toolkits/IItemDefinitionApp.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemDefinitionAppAssetType::Construct(
	const FArguments& InArgs,
	const TSharedPtr<IItemDefinitionApp>& InApp)
{
	WeakApp = InApp;

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
							.Image(this, &ThisClass::HandleGetAssetTypeIcon)
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

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FText SItemDefinitionAppAssetType::HandleGetAssetTypeText() const
{
	TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin();
	if (!AppPtr.IsValid()) return FText();

	const UItemDefinitionBase* ItemDef = AppPtr->GetItemDefinition();
	if (ItemDef == nullptr)
	{
		return FText::FromString(TEXT("None"));
	}

	return ItemDef->GetClass()->GetDisplayNameText();
}

const FSlateBrush* SItemDefinitionAppAssetType::HandleGetAssetTypeIcon() const
{
	TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin();
	if (!AppPtr.IsValid()) return nullptr;

	const UItemDefinitionBase* ItemDef = AppPtr->GetItemDefinition();
	if (ItemDef == nullptr)
	{
		return FSlateIconFinder::FindIconBrushForClass(UItemDefinitionBase::StaticClass());
	}

	return FSlateIconFinder::FindIconBrushForClass(ItemDef->GetClass());
}

UClass* SItemDefinitionAppAssetType::GetAssetClass() const
{
	TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin();
	if (!AppPtr.IsValid()) return nullptr;

	const UItemDefinitionBase* ItemDef = AppPtr->GetItemDefinition();
	if (ItemDef == nullptr)
	{
		return UItemDefinitionBase::StaticClass();
	}

	return ItemDef->GetClass();
}
