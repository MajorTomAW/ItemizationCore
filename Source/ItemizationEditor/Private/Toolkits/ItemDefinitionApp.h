// Author: Tom Werner (MajorT), 2025 November

#pragma once
#include "Toolkits/IItemDefinitionApp.h"

class SItemDataListView;
class SItemDefinitionAppViewport;
class FItemDefinitionViewModel;
class FStandaloneItemDefinitionAppHost;

class FItemDefinitionApp
	: public IItemDefinitionApp
	, public FSelfRegisteringEditorUndoClient
	, public FGCObject
	, public FNotifyHook
{
	using ThisClass = FItemDefinitionApp;

public:
	FItemDefinitionApp();

	void InitApp(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UItemDefinitionBase* InItemDefinition);


	//~ Begin IItemDefinitionApp Interface
	virtual UItemDefinitionBase* GetItemDefinition() const override;

	/** Get the list of modes that were registered with the application. */
	virtual TArray<FName> GetApplicationModes() const override;

	/** Get the delegate called when the active application mode changes. */
	virtual FOnAppModeChanged& OnAppModeChanged() override { return AppModeChangedDelegate; }

	virtual void AddItemDefinitionAppMode(UE::ItemizationEditor::FItemDefinitionAppModeInfo ModeArgs, TSharedRef<FItemDefinitionAppMode> ApplicationMode) override;
	virtual void RemoveItemDefinitionAppMode(FName ModeName) override;

	virtual TOptional<UE::ItemizationEditor::FItemDefinitionAppModeInfo> GetModeInfo(FName ModeName) const override;
	//~ End IItemDefinitionApp Interface

	//~ Begin FWorkflowCentricApplication Interface
	virtual void AddApplicationMode(FName ModeName, TSharedRef<FApplicationMode> Mode) override;
	virtual void SetCurrentMode(FName NewMode) override;
	//~ End FWorkflowCentricApplication Interface

	//~ Begin FNotifyHook Interface
	virtual void NotifyPreChange(FProperty* PropertyAboutToChange) override;
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
	//~ End FNotifyHook Interface

	//~ Begin FAssetEditorToolkit Interface
	virtual void PostInitAssetEditor() override;
	virtual void OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit) override;
	virtual void OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit) override;
	//~ End FAssetEditorToolkit Interface

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

	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

	TSharedPtr<IDetailsView> GetAssetDetailsView() { return AssetDetailsView; }
	TSharedPtr<IDetailsView> GetSettingsDetailsView() { return SettingsDetailsView; }
	TSharedPtr<IStructureDetailsView> GetDataListDetailsView() { return DataListDetailsView; }

protected:
	//~ Begin FGCObject Interface
	virtual FString GetReferencerName() const override { return TEXT("FItemDefinitionEditor"); }
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

	void OnModularFeatureRegistered(const FName& Type, class IModularFeature* ModularFeature);

	void OnDataListFinishedChangingProperty(const FPropertyChangedEvent& ChangedEvent);

	void CreateInternalWidgets();

public:
	TSharedRef<SWidget> SpawnTab_AssetDetails(const FSpawnTabArgs& Args);
	TSharedRef<SWidget> SpawnTab_Settings(const FSpawnTabArgs& Args);
	TSharedRef<SWidget> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SWidget> SpawnTab_DataList(const FSpawnTabArgs& Args);
	TSharedRef<SWidget> SpawnTab_DataListDetails(const FSpawnTabArgs& Args);

private:
	TSharedRef<IDetailCustomization> GetAssetDetailsCustomization();
	TSharedRef<IDetailCustomization> GetSettingsDetailsCustomization();

private:
	/** The asset property details view. */
	TSharedPtr<IDetailsView> AssetDetailsView;
	TSharedPtr<IDetailsView> SettingsDetailsView;
	TSharedPtr<SItemDefinitionAppViewport> Viewport;
	TSharedPtr<SItemDataListView> DataListView;
	TSharedPtr<IStructureDetailsView> DataListDetailsView;


	TObjectPtr<UItemDefinitionBase> ItemDefinition = nullptr;

	TSharedPtr<FItemDefinitionViewModel> ItemViewModel;

	TSharedRef<FUICommandList> DataListCommands;
	TSharedPtr<IToolkit> HostedToolkit;
	TSharedPtr<FWorkspaceItem> WorkspaceMenuCategory;
	TSharedPtr<FStandaloneItemDefinitionAppHost> AppHost;

	/** Map of mode name to cached mode info. */
	TMap<FName, UE::ItemizationEditor::FItemDefinitionAppModeInfo> CachedModeInfo;

	/** Delegate called when the active application mode changes. */
	FOnAppModeChanged AppModeChangedDelegate;
};
