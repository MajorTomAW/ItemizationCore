// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


class FItemDataUIInfo;
class FItemDefinitionViewModel;

class SItemDataListView : public SCompoundWidget
{
	using ThisClass = SItemDataListView;
public:
	SLATE_BEGIN_ARGS(SItemDataListView)
		{
		}

	SLATE_END_ARGS()

	SItemDataListView();
	virtual ~SItemDataListView() override;

public:
	void Construct(const FArguments& InArgs, TSharedRef<FItemDefinitionViewModel> InItemViewModel, const TSharedRef<FUICommandList>& InCommandList);

	TSharedPtr<FItemDefinitionViewModel> GetViewModel() const;
	void SetSelection(const TArray<TSharedPtr<FItemDataUIInfo>>& SelectedData) const;

	void BindCommands();

private:
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void UpdateList();

	/** ViewModel handlers. */
	void HandleModelAssetChanged();
	void HandleModelItemDataChanged(const TSharedPtr<FItemDataUIInfo>& AffectedItemData, const FPropertyChangedEvent& ChangedEvent);
	void HandleModelItemDataAdded(TSharedPtr<FItemDataUIInfo>& AddedItemData);
	void HandleModelItemDataRemoved();
	void HandleModelSelectionChanged(const TArray<TSharedPtr<FItemDataUIInfo>>& Selection);

	/** ListView handlers. */
	TSharedRef<ITableRow> HandleGenerateRow(TSharedPtr<FItemDataUIInfo> InUIInfo, const TSharedRef<STableViewBase>& InOwnerTableView);
	void HandleListSelectionChanged(TSharedPtr<FItemDataUIInfo> InSelectedItem, ESelectInfo::Type SelectionType);

	/** ComboButton handlers. */
	TSharedRef<SWidget> HandleGenerateStructPicker();
	void HandleStructPicked(const UScriptStruct* InStruct);

	TSharedRef<SWidget> HandleGenerateSettingsMenu();
	TSharedPtr<SWidget> HandleListContextMenuOpening();

	/** Command handlers. */
	bool HasSelection() const;

	void HandleDeleteItemData();

private:
	TSharedPtr<SComboButton> ComboButton;
	TSharedPtr<FItemDefinitionViewModel> ItemViewModel;
	TSharedPtr<SListView<TSharedPtr<FItemDataUIInfo>>> ListView;
	TSharedPtr<SScrollBar> ExternalScrollBar;
	TSharedPtr<SScrollBox> ViewBox;

	TArray<TSharedPtr<FItemDataUIInfo>> ItemDataList;
	TSharedPtr<FUICommandList> CommandList;

	bool bItemsDirty;
	bool bUpdatingSelection;
};
