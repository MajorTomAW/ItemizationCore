// Copyright © 2025 MajorT. All Rights Reserved.


#include "SItemAssetTypePicker.h"

#include "ItemizationCoreSettings.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/SListView.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

///////////////////////////////////////////////////////////////////
/// SItemAssetTypePicker

void SItemAssetTypePicker::Construct(const FArguments& InArgs)
{
	OnItemAssetTypePicked = InArgs._OnItemAssetTypePickedDelegate;
	TextFilterPtr = MakeShared<FTextFilterExpressionEvaluator>(ETextFilterExpressionEvaluatorMode::BasicString);
	bNeedsRefresh = true;

	SAssignNew(AssetTypeList, SListView<TSharedPtr<FName>>)
		.SelectionMode(ESelectionMode::Single)
		.ListItemsSource(&AssetTypeItems)
		.OnGenerateRow(this, &SItemAssetTypePicker::OnGenerateRow)
		.OnSelectionChanged(this, &SItemAssetTypePicker::OnSelectionChanged)
		.HeaderRow
		(
			SNew(SHeaderRow)
			.Visibility(EVisibility::Collapsed)
			+ SHeaderRow::Column(TEXT("Type"))
			.DefaultLabel(NSLOCTEXT("AssetTypePicker", "Type", "Asset Type"))
		);

	TSharedRef<SListView<TSharedPtr<FName>>> AssetTypeListView = AssetTypeList.ToSharedRef();
	TSharedPtr<SWidget> Content =
		SNew(SBox)
		.MaxDesiredWidth(640.f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
			
				// Search Box
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(2.f)
					[
						SAssignNew(SearchBox, SSearchBox)
						.OnTextChanged(this, &SItemAssetTypePicker::OnFilterTextChanged)
						.OnTextCommitted(this, &SItemAssetTypePicker::OnFilterTestCommitted)
					]
				]

				// List View
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						AssetTypeListView
					]
				]
			]
		];

	
	this->ChildSlot
	[
		Content.ToSharedRef()	
	];
}

void SItemAssetTypePicker::Tick(
	const FGeometry& AllottedGeometry,
	const double InCurrentTime,
	const float InDeltaTime)
{
	if (bNeedsRefresh)
	{
		bNeedsRefresh = false;
		Populate();
	}
}

TSharedRef<ITableRow> SItemAssetTypePicker::OnGenerateRow(
	TSharedPtr<FName> InItem,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedRef<SItemAssetType> ReturnRow = SNew(SItemAssetType, OwnerTable)
		.AssetTypeDisplayName(FText::FromName(*InItem));
	
	return ReturnRow;
}

void SItemAssetTypePicker::OnSelectionChanged(
	TSharedPtr<FName> InItem,
	ESelectInfo::Type SelectionInfo)
{
	if (SelectionInfo == ESelectInfo::OnNavigation)
	{
		return;
	}

	OnItemAssetTypePicked.ExecuteIfBound(*InItem);
}

void SItemAssetTypePicker::OnFilterTextChanged(const FText& InFilterText)
{
	// Update the filter text
	TextFilterPtr->SetFilterText(InFilterText);
	SearchBox->SetError(TextFilterPtr->GetFilterErrorText());

	// Mark dirty
	bNeedsRefresh = true;
}

void SItemAssetTypePicker::OnFilterTestCommitted(const FText& InFilterText, ETextCommit::Type CommitInfo)
{
	if (CommitInfo == ETextCommit::OnEnter)
	{
		TArray<TSharedPtr<FName>> Selection = AssetTypeList->GetSelectedItems();
		if (Selection.Num() > 0)
		{
			TSharedPtr<FName> First = Selection[0];
			OnItemAssetTypePicked.ExecuteIfBound(*First);
		}
	}
}

void SItemAssetTypePicker::Populate()
{
	AssetTypeItems.Empty();
	for (const FName& Name : UItemizationCoreSettings::Get()->ItemTypes)
	{
		if (TextFilterPtr->TestTextFilter(FBasicStringFilterExpressionContext(Name.ToString())) == false)
		{
			continue;
		}
		
		AssetTypeItems.Add(MakeShared<FName>(Name));
	}

	AssetTypeList->RebuildList();
}


///////////////////////////////////////////////////////////////////
/// SItemAssetType

void SItemAssetType::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
{
	this->ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.Padding(6.f, 3.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(InArgs._AssetTypeDisplayName)
		]
	];

	ConstructInternal(
		STableRow::FArguments()
			.ShowSelection(true),
		InOwnerTable);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
