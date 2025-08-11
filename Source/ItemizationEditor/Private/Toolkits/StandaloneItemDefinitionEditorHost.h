// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Toolkits/IItemDefinitionEditorHost.h"

class FItemDefinitionApplication;

class FStandaloneItemDefinitionEditorHost : public IItemDefinitionEditorHost
{
public:
	void Init(const TWeakPtr<FItemDefinitionApplication>& InWeakEditor);

	//~ Begin IItemDefinitionEditorHost Interface
	virtual bool CanToolkitSpawnWorkspaceTab() const override;
	
	virtual UItemDefinitionBase* GetItemDefinition() const override;

	virtual FSimpleMulticastDelegate& OnItemDefinitionChanged() override;

	virtual TSharedPtr<IDetailsView> GetAssetDetailsView() const override;
	virtual TSharedPtr<IDetailsView> GetDetailsView() const override;

	virtual TSharedPtr<UE::ItemizationEditor::FWorkspaceTabHost> GetTabHost() const override;
	//~ End IItemDefinitionEditorHost Interface

protected:
	TWeakPtr<FItemDefinitionApplication> WeakEditor;
	FSimpleMulticastDelegate OnItemDefinitionChangedDelegate;
	TSharedPtr<UE::ItemizationEditor::FWorkspaceTabHost> TabHost;
};
