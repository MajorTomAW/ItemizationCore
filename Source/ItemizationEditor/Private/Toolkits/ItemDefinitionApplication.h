// Author: Tom Werner (MajorT), 2025

#pragma once
#include "ItemDefinitionViewModel.h"
#include "Toolkits/IItemDefinitionApplication.h"

class FItemDefinitionEditorModeUILayer;
class FStandaloneItemDefinitionEditorHost;
class IItemDefinitionEditorHost;
class SItemComponentDataView;
class UItemDefinitionBase;

class FItemDefinitionApplication
	: public IItemDefinitionApplication
	, public FEditorUndoClient
	, public FGCObject
{
	using ThisClass = FItemDefinitionApplication;
public:
	FItemDefinitionApplication();

	/** Initializes the editor for the given Item Definition. */
	void InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UItemDefinitionBase* ItemDefinition);

	/** Returns the Item Definition being edited. */
	virtual UItemDefinitionBase* GetItemDefinition() const override { return ItemDefinition; }

	TSharedPtr<IDetailsView> GetSelectionDetailsView() const { return SelectionDetailsView; }
	TSharedPtr<IDetailsView> GetAssetDetailsView() const { return AssetDetailsView; }


	//~ Begin IItemDefinitionApplication Interface
	virtual void AddItemDefinitionAppMode(UE::ItemizationEditor::FItemDefinitionAppModeInfo ModeArgs, TSharedRef<FItemDefinitionApplicationMode> ApplicationMode) override;
	virtual void RemoveItemDefinitionAppMode(FName ModeName) override;

	virtual TOptional<UE::ItemizationEditor::FItemDefinitionAppModeInfo> GetModeInfo(FName ModeName) const override;
	//~ End IItemDefinitionApplication Interface

	//~ Begin FWorkflowCentricApplication Interface
	virtual void AddApplicationMode(FName ModeName, TSharedRef<FApplicationMode> Mode) override;
	virtual void SetCurrentMode(FName NewMode) override;
	//~ End FWorkflowCentricApplication Interface

	/** Get the list of modes that were registered with the application. */
	virtual TArray<FName> GetApplicationModes() const override;

	/** Get the delegate called when the active application mode changes. */
	virtual FOnAppModeChanged& OnAppModeChanged() override { return AppModeChangedDelegate; }

protected:
	//~ Begin IToolkit Interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	virtual void BrowseDocumentation_Execute() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;

	virtual void OnClose() override;
	//~ End IToolkit Interface

	//~ Begin FAssetEditorToolkit Interface
	virtual void PostInitAssetEditor() override;
	virtual void OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit) override;
	virtual void OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit) override;
	//~ End FAssetEditorToolkit Interface

	//~ Begin FGCObject Interface
	virtual FString GetReferencerName() const override { return TEXT("FItemDefinitionEditor"); }
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

	//~ Begin FNotifyHook Interface
	virtual void NotifyPreChange(FProperty* PropertyAboutToChange) override;
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
	//~ End FNotifyHook Interface

	void OnModularFeatureRegistered(const FName& Type, class IModularFeature* ModularFeature);


	void CreateInternalWidgets();


public:
	/** Spawns the tab for the details view. */
	TSharedRef<SWidget> SpawnTab_AssetDetails(const FSpawnTabArgs& Args);

	/** Spawns the tab for the component data. */
	TSharedRef<SWidget> SpawnTab_ComponentData(const FSpawnTabArgs& Args);

	/** Spawns the tab for the selection details view. */
	TSharedRef<SWidget> SpawnTab_SelectionDetails(const FSpawnTabArgs& Args);

	/** Spawns the tab for the developer details view. */
	TSharedRef<SWidget> SpawnTab_DeveloperDetails(const FSpawnTabArgs& Args);


private:
	/** Creates and initializes the toolbar for this editor. */
	void RegisterToolbar() const;

	/** Creates and initializes menus for this editor. */
	void RegisterMenus() const;

private:
	/** The toolkit host for this editor. */
	TSharedPtr<IToolkit> HostedToolkit;

	/** The hoster interface for this editor. */
	TSharedPtr<FStandaloneItemDefinitionEditorHost> EditorHost;

	/** */
	TSharedPtr<FItemDefinitionEditorModeUILayer> ModeUILayer;


	/** Item Definition being edited */
	TObjectPtr<UItemDefinitionBase> ItemDefinition = nullptr;

	/** The selection details view. */
	TSharedPtr<IDetailsView> SelectionDetailsView;
	/** The asset property details view. */
	TSharedPtr<IDetailsView> AssetDetailsView;

	/** The item component data view. */
	TSharedPtr<SItemComponentDataView> ItemComponentDataView;

	/** Viewmodel for the item definition. */
	TSharedPtr<FItemDefinitionViewModel> ItemDefinitionViewModel;


	/** Map of mode name to cached mode info. */
	TMap<FName, UE::ItemizationEditor::FItemDefinitionAppModeInfo> CachedModeInfo;

	/** Delegate called when the active application mode changes. */
	FOnAppModeChanged AppModeChangedDelegate;
};
