// Copyright © 2025 Playton. All Rights Reserved.


#include "SItemComponentDataView.h"

#include "SItemComponentDataRow.h"
#include "SlateOptMacros.h"
#include "SPositiveActionButton.h"
#include "Toolkits/ItemDefinitionViewModel.h"

#define LOCTEXT_NAMESPACE "ItemizationEditor"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemComponentDataView::Construct(const FArguments& InArgs, TSharedRef<FItemDefinitionViewModel> ItemDefinitionViewModel)
{
	ItemViewModel = ItemDefinitionViewModel;
	
	TSharedRef<SScrollBar> HorizontalScrollBar = SNew(SScrollBar)
		.Orientation(Orient_Horizontal)
		.Thickness(FVector2D(12.0f, 12.0f));

	TSharedRef<SScrollBar> VerticalScrollBar = SNew(SScrollBar)
		.Orientation(Orient_Vertical)
		.Thickness(FVector2D(12.0f, 12.0f));

	Components = ItemDefinitionViewModel->GetDataList();


	ComponentView = SNew(SListView<TSharedPtr<FItemComponentInstanceListEntry> >)
		.ListItemsSource(&Components)
		.ExternalScrollbar(VerticalScrollBar)
		.AllowOverscroll(EAllowOverscroll::Yes)
		.OnGenerateRow(this, &SItemComponentDataView::HandleGenerateComponentRow);
	
	this->ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(2.f)
			[
				SNew(SHorizontalBox)

				// Add Component
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(4.f, 2.f)
				.AutoWidth()
				[
					SNew(SPositiveActionButton)
					.ToolTipText(LOCTEXT("AddItemComponentDataTooltip", "Add a new component data to the item."))
					.Icon(FAppStyle::Get().GetBrush("Icons.Plus"))
					.Text(LOCTEXT("AddItemComponentData", "Add Component"))
					.OnClicked(this, &SItemComponentDataView::HandleAddComponentDataClicked)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(SSpacer)
				]
			]
		]
		
		+ SVerticalBox::Slot()
		.Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(0.f)
			[
				SAssignNew(ViewBox, SScrollBox)
				.Orientation(Orient_Horizontal)
				.ExternalScrollbar(HorizontalScrollBar)
				+ SScrollBox::Slot()
				.FillSize(1.f)
				[
					ComponentView.ToSharedRef()
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				VerticalScrollBar
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			HorizontalScrollBar
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SItemComponentDataView::HandleAddComponentDataClicked()
{
	return FReply::Handled();
}

TSharedRef<ITableRow> SItemComponentDataView::HandleGenerateComponentRow(
	TSharedPtr<FItemComponentInstanceListEntry>  InInstance,
	const TSharedRef<STableViewBase>& InOwnerTableView)
{
	return SNew(SItemComponentDataRow, InOwnerTableView, InInstance, ViewBox);
}


#undef LOCTEXT_NAMESPACE
