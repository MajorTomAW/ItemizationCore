// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "Toolkits/Slate/ItemDataUIInfo.h"

class ITEMIZATIONEDITOR_API FItemDefinitionViewModel
	: public FEditorUndoClient
	, public TSharedFromThis<FItemDefinitionViewModel>
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnAssetChanged)
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemDataChanged, const TSharedPtr<FItemDataUIInfo>& /*AffectedItemData*/, const FPropertyChangedEvent& /*ChangedEvent*/)
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemDataAdded, TSharedPtr<FItemDataUIInfo>& /*AddedItemData*/)
	DECLARE_MULTICAST_DELEGATE(FOnItemDataRemoved)
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnSelectionChanged, const TArray<TSharedPtr<FItemDataUIInfo>>& /*Selection*/)

	FItemDefinitionViewModel();
	virtual ~FItemDefinitionViewModel() override;

	void Init(UItemDefinitionBase* InItemDefinition);

	UItemDefinitionBase* GetItemDefinition() const;
	void GetDataList(TArray<TSharedPtr<FItemDataUIInfo>>& OutDataList) const;

	FOnAssetChanged& GetOnAssetChanged() { return OnAssetChanged; }
	FOnItemDataChanged& GetOnItemDataChanged() { return OnItemDataChanged; }
	FOnItemDataAdded& GetOnItemDataAdded() { return OnItemDataAdded; }
	FOnItemDataRemoved& GetOnItemDataRemoved() { return OnItemDataRemoved; }
	FOnSelectionChanged& GetOnSelectionChanged() { return OnSelectionChanged; }

	void ClearSelection();
	void SetSelection(const TArray<TSharedPtr<FItemDataUIInfo>>& InSelection);
	const TArray<TSharedPtr<FItemDataUIInfo>>& GetSelection() const;
	bool IsSelected(const TSharedPtr<FItemDataUIInfo>& InItemData) const;
	bool HasSelection() const;
	void AddItemData(const UScriptStruct* InStruct);
	void RemoveSelectedItemData();

protected:
	//~ Begin FEditorUndoClient Interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	//~ End FEditorUndoClient Interface

protected:
	TWeakObjectPtr<UItemDefinitionBase> ItemDefinition;
	TArray<TSharedPtr<FItemDataUIInfo>> SelectedItemData;

	FOnAssetChanged OnAssetChanged;
	FOnItemDataChanged OnItemDataChanged;
	FOnItemDataAdded OnItemDataAdded;
	FOnItemDataRemoved OnItemDataRemoved;
	FOnSelectionChanged OnSelectionChanged;
};
