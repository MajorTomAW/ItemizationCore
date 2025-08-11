// Author: Tom Werner (MajorT), 2025

#pragma once

class FItemComponentInstanceListEntry;
struct FItemComponentDataInstance;
class UItemDefinitionBase;

class FItemDefinitionViewModel
	: public FEditorUndoClient
	, public  TSharedFromThis<FItemDefinitionViewModel>
{
public:
	FItemDefinitionViewModel();
	virtual ~FItemDefinitionViewModel() override;

	TArray<TSharedPtr<FItemComponentInstanceListEntry> > GetDataList() const;

	void Init(UItemDefinitionBase* InItemDefinition);

protected:
	//~ Begin FEditorUndoClient Interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	//~ End FEditorUndoClient Interface


protected:
	TWeakObjectPtr<UItemDefinitionBase> ItemDefinition;
};
