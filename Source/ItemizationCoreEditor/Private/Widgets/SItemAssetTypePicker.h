// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemizationCoreEditor.h"
#include "Widgets/SCompoundWidget.h"


/** The custom item asset type picker widget. */
class SItemAssetTypePicker : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SItemAssetTypePicker)
		{
		}
	SLATE_ARGUMENT(FOnItemAssetTypePicked, OnItemAssetTypePickedDelegate)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

protected:
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FName> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnSelectionChanged(TSharedPtr<FName> InItem, ESelectInfo::Type SelectionInfo);

	void OnFilterTextChanged(const FText& InFilterText);
	void OnFilterTestCommitted(const FText& InFilterText, ETextCommit::Type CommitInfo);

	void Populate();

private:
	TSharedPtr<FTextFilterExpressionEvaluator> TextFilterPtr;
	TSharedPtr<SSearchBox> SearchBox;
	TSharedPtr<SListView<TSharedPtr<FName>>> AssetTypeList;
	TArray<TSharedPtr<FName>> AssetTypeItems;
	
	FOnItemAssetTypePicked OnItemAssetTypePicked;

	uint8 bNeedsRefresh : 1;
};

/** The row widget to generate for the item asset type picker. */
class SItemAssetType : public STableRow<TSharedPtr<FName>>
{
public:
	SLATE_BEGIN_ARGS(SItemAssetType)
		{
		}
	SLATE_ARGUMENT(FText, AssetTypeDisplayName)	
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable);
};