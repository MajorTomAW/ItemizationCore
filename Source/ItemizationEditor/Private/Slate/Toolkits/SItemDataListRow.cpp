// Author: Tom Werner (MajorT), 2025 November


#include "SItemDataListRow.h"

#include "ItemDefinitionAppUserSettings.h"
#include "SlateOptMacros.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Styling/SlateIconFinder.h"
#include "Toolkits/ItemDefinitionViewModel.h"
#include "Widgets/Text/SRichTextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemDataListRow::Construct(
	const FArguments& InArgs,
	const TSharedRef<STableViewBase>& InOwnerTableView,
	TSharedPtr<FItemDataUIInfo> InItemData,
	const TSharedPtr<SScrollBox>& InViewBox,
	TSharedPtr<FItemDefinitionViewModel> InItemViewModel)
{
	ItemData = InItemData;
	ViewModel = InItemViewModel;

	ConstructInternal(STableRow::FArguments()
		.Style(&FItemizationEditorStyle::Get().GetWidgetStyle<FTableRowStyle>("ItemData.Selection")),
		InOwnerTableView);

	this->ChildSlot
	.HAlign(HAlign_Fill)
	[
		SNew(SBox)
		.MinDesiredHeight(32.f)
		.Padding(FMargin(12.f, 2.f, 2.f, 8.f))
		[
			SNew(SHorizontalBox)

			/*+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SLine, SharedThis(this))
				.IndentAmount(12.f)
				.ShouldDrawWires(true)
				.Visibility(EVisibility::SelfHitTestInvisible)
			]*/

			// Component Box
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(32.f)
					.MinDesiredWidth(256.f)
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Left)
					[
						SNew(SBorder)
						.BorderImage(FItemizationEditorStyle::Get().GetBrush("ItemComponent.Data.Border"))
						.BorderBackgroundColor(this, &ThisClass::GetSelectedColor)
						[
							SNew(SBorder)
							.BorderImage(FItemizationEditorStyle::Get().GetBrush("ItemComponent.Data"))
							.BorderBackgroundColor(FSlateColor(FColor::FromHex("#1F97A760")))
							.Padding(FMargin(12.f, 0.f, 12.f, 0.f))
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SNew(SHorizontalBox)

									// Icon
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									.Padding(0.f, 0.f, 4.f, 0.f)
									[
										SNew(SImage)
										.Image(this, &ThisClass::GetComponentIcon)
									]

									+ SHorizontalBox::Slot()
									.VAlign(VAlign_Center)
									.HAlign(HAlign_Left)
									.AutoWidth()
									.Padding(FMargin(0.f, 0.f, 0.f, 0.f))
									[
										SNew(STextBlock)
										.Text(this, &ThisClass::GetComponentDisplayText)
										.TextStyle(FItemizationEditorStyle::Get(), "ItemData.Title")
									]
								]
							]
						]
					]
				]

				// Description
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(12.f, 2.f, 0.f, 0.f))
				[
					SNew(SBorder)
					.Visibility_Lambda([]()
					{
						return UItemDefinitionAppUserSettings::Get()->bShowItemDataDescriptions
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
					.VAlign(VAlign_Center)
					.BorderImage(FAppStyle::GetNoBrush())
					.Padding(0.f)
					[
						SNew(SRichTextBlock)
						.Text(this, &ThisClass::GetComponentDesc)
						.TextStyle(&FItemizationEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("ItemData.Description"))
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::OnDemand)
						//@TODO:+ SRichTextBlock::Decorator(FTextStyleDecorator())
					]
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

const FSlateBrush* SItemDataListRow::GetComponentIcon() const
{
	// NO custom icons lol
	/*if (ItemData.IsValid() && ItemData->DataInstance && ItemData->DataInstance->Component.IsValid())
	{
		if (const UScriptStruct* ScriptStruct = ItemData->DataInstance->Component.GetScriptStruct())
		{
			const FSlateBrush* Brush = nullptr;
			if (UStruct* OwnerStruct = ScriptStruct->GetOwnerStruct())
			{
				Brush = FItemizationEditorStyle::Get().GetBrush(OwnerStruct->GetFName());
			}

			if (Brush == nullptr ||
				!Brush->IsSet() ||
				Brush->GetResourceName().ToString().EndsWith(TEXT("Checkerboard.png")))
			{
				Brush = FSlateIconFinder::FindIconBrushForClass(ScriptStruct->GetClass());
			}

			return Brush;
		}
	}*/

	return FSlateIconFinder::FindIconBrushForClass(UScriptStruct::StaticClass());
}

FText SItemDataListRow::GetComponentDisplayText() const
{
	if (ItemData.IsValid() && !ItemData->LabelText.IsEmpty())
	{
		return ItemData->LabelText;
	}

	return FText::FromString(TEXT("None"));
}




FText SItemDataListRow::GetComponentDesc() const
{
	if (ItemData.IsValid() && ItemData->DataInstance)
	{
		if (const FItemComponentData* DataPtr = ItemData->DataInstance->GetComponent<FItemComponentData>())
		{
			return DataPtr->GetDescription();
		}
	}

	return FText::GetEmpty();
}

FSlateColor SItemDataListRow::GetSelectedColor() const
{
	if (ViewModel && ViewModel->IsSelected(ItemData))
	{
		return FStyleColors::Primary;
	}

	return FLinearColor::Transparent;
}
