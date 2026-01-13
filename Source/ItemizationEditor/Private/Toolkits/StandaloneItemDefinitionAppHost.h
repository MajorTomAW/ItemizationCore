// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "Toolkits/IItemDefinitionAppHost.h"

class FItemDefinitionApp;

class FStandaloneItemDefinitionAppHost
	: public IItemDefinitionAppHost
{
public:
	void Init(const TWeakPtr<FItemDefinitionApp>& InApp);

	//~ Begin IItemDefinitionEditorHost Interface
	virtual bool CanToolkitSpawnWorkspaceTab() const override;

	virtual UItemDefinitionBase* GetItemDefinition() const override;

	virtual FSimpleMulticastDelegate& OnItemDefinitionChanged() override;

	virtual TSharedPtr<IDetailsView> GetAssetDetailsView() const override;

	virtual TSharedPtr<UE::ItemizationEditor::FWorkspaceTabHost> GetTabHost() const override;
	//~ End IItemDefinitionEditorHost Interface

protected:
	TWeakPtr<FItemDefinitionApp> WeakApp;
	FSimpleMulticastDelegate OnItemDefinitionChangedDelegate;
	TSharedPtr<UE::ItemizationEditor::FWorkspaceTabHost> TabHost;
};
