// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemizationCoreEditor.h"
#include "ItemizationEditorToolbar.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"

class SItemizationEditorViewport;
class UItemDefinition;

class ITEMIZATIONCOREEDITOR_API FItemizationEditorApplication : public FWorkflowCentricApplication, public FSelfRegisteringEditorUndoClient, public FNotifyHook, public FGCObject
{
public:
	FItemizationEditorApplication();
	virtual void InitItemizationEditorApp(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UItemDefinition* InItem);

	TSharedRef<SWidget> SpawnTab_Details() const;
	TSharedRef<SWidget> SpawnTab_DisplayInfo() const;
	TSharedRef<SWidget> SpawnTab_Placement() const;
	TSharedRef<SWidget> SpawnTab_Development() const;
	TSharedRef<SWidget> SpawnTab_Components() const;
	TSharedRef<SWidget> SpawnTab_Equipment() const;
	TSharedRef<SWidget> SpawnTab_Viewport() const;

	TSharedPtr<IDetailsView> GetDetailsView() const { return DetailsView; }
	TSharedPtr<IDetailsView> GetDisplayInfoView() const { return DisplayInfoView; }
	TSharedPtr<IDetailsView> GetPlacementView() const { return PlacementView; }
	TSharedPtr<IDetailsView> GetDevelopmentView() const { return DevelopmentView; }
	TSharedPtr<IDetailsView> GetComponentsView() const { return ComponentsView; }
	TSharedPtr<IDetailsView> GetEquipmentView() const { return EquipmentView; }
	TSharedPtr<SItemizationEditorViewport> GetViewport() const { return Viewport; }

	FSimpleDelegate& GetOnAssetSavedDelegate() { return OnAssetSavedDelegate; }
	UItemDefinition* GetItemDefinition() const { return ItemDefinition; }
	const TMap<FName, TSharedPtr<FApplicationMode>>& GetRegisteredApplicationModes() const { return RegisteredApplicationModes; }
	TSharedPtr<FItemizationEditorToolbar> GetToolbarBuilder() const { return ToolbarBuilder; }

	bool CanActivateMode(FName ModeId) const;
	bool CanShowMode(FName ModeId) const;

protected:
	//~ Begin IToolkit Interface
	virtual FName GetToolkitFName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	virtual void BrowseDocumentation_Execute() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual void PostRegenerateMenusAndToolbars() override;
	virtual void PostInitAssetEditor() override;

	virtual void SaveAsset_Execute() override;
	//~ End IToolkit Interface
	
	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("FItemizationEditorApplication"); }
	//~ End FGCObject Interface

	//~ Begin FNotifyHook Interface
	virtual void NotifyPreChange(FProperty* PropertyAboutToChange) override;
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
	//~ End FNotifyHook Interface

	//~ Begin FEditorUndoClient Interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	//~ End FEditorUndoClient Interface

private:
	void CreateInternalWidgets();

	TSharedRef<IDetailCustomization> GetDetailsCustomization();
	TSharedRef<IDetailCustomization> GetDisplayInfoCustomization();
	TSharedRef<IDetailCustomization> GetPlacementCustomization();
	TSharedRef<IDetailCustomization> GetDevelopmentCustomization();
	TSharedRef<IDetailCustomization> GetComponentsCustomization();
	TSharedRef<IDetailCustomization> GetEquipmentCustomization();

private:
	FSimpleDelegate OnAssetSavedDelegate;
	
	TMap<FName, TSharedPtr<FApplicationMode>> RegisteredApplicationModes;
	TObjectPtr<UItemDefinition> ItemDefinition;
	TSharedPtr<FItemizationEditorToolbar> ToolbarBuilder;

	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<IDetailsView> DisplayInfoView;
	TSharedPtr<IDetailsView> PlacementView;
	TSharedPtr<IDetailsView> DevelopmentView;
	TSharedPtr<IDetailsView> ComponentsView;
	TSharedPtr<IDetailsView> EquipmentView;
	TSharedPtr<SItemizationEditorViewport> Viewport;
};

