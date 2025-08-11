// Author: Tom Werner (MajorT), 2025


#include "Toolkits/ItemDefinitionApplication.h"

#include "ContextObjectStore.h"
#include "EditorModeManager.h"
#include "IDocumentation.h"
#include "ItemDefinitionEditorUILayer.h"
#include "ItemizationEditorModule.h"
#include "StandaloneItemDefinitionEditorHost.h"
#include "Framework/Docking/LayoutExtender.h"
#include "Items/ItemDefinitionBase.h"
#include "Modes/ItemDefinitionAppMode_Default.h"
#include "Slate/Toolkits/SItemComponentDataView.h"
#include "Slate/Widgets/SItemDefinitionEditorModeSwitcher.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/AssetEditorToolkitMenuContext.h"
#include "Toolkits/GlobalEditorCommonCommands.h"
#include "Toolkits/ItemDefinitionApplicationMode.h"
#include "Toolkits/ItemDefinitionEditorWorkspaceTabHost.h"
#include "Tools/EdModeInteractiveToolsContext.h"
#include "Editor/UnrealEd/Private/Toolkits/AssetEditorCommonCommands.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionEditor"

FItemDefinitionApplication::FItemDefinitionApplication()
	: IItemDefinitionApplication()
{
}

void FItemDefinitionApplication::InitEditor(
	const EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost,
	UItemDefinitionBase* InItemDefinition)
{
	using namespace UE::ItemizationEditor;
	ItemDefinition = InItemDefinition;
	check(IsValid(ItemDefinition));

	EditorHost = MakeShared<FStandaloneItemDefinitionEditorHost>();
	EditorHost->Init(StaticCastSharedRef<FItemDefinitionApplication>(AsShared()));

	ItemDefinitionViewModel = MakeShared<FItemDefinitionViewModel>();
	ItemDefinitionViewModel->Init(ItemDefinition);

	TSharedPtr<ThisClass> ThisPtr(SharedThis(this));

	CreateEditorModeManager();
	CreateInternalWidgets();

	//@TODO: Maybe link command list to in editor commands?

	const TSharedRef<FTabManager::FLayout> DummyLayout = FTabManager::NewLayout("NullLayout")->AddArea(FTabManager::NewPrimaryArea());
	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar = true;

	// Initialize the asset editor with a placeholder layout, as the actual layouts will be provided by the application modes.
	FAssetEditorToolkit::InitAssetEditor(
		Mode,
		InitToolkitHost,
		Ids::AppId,
		DummyLayout,
		bCreateDefaultStandaloneMenu,
		bCreateDefaultToolbar,
		ItemDefinition);

	// Register application modes using the modular feature system.
	for (IItemDefinitionApplicationModeFactory* Factory : IModularFeatures::Get()
		.GetModularFeatureImplementations<IItemDefinitionApplicationModeFactory>(IItemDefinitionApplicationModeFactory::ModularFeatureName))
	{
		TSharedRef<FItemDefinitionApplicationMode> ApplicationMode =
			Factory->CreateItemDefinitionApplicationMode(SharedThis<ThisClass>(this));

		AddItemDefinitionAppMode(ApplicationMode->GetModeInfo(), MoveTemp(ApplicationMode));
	}

	//RegisterMenus();
	//RegisterToolbar();

	FLayoutExtender LayoutExtender;
	IItemizationEditorModule& ItemizationEditor = IItemizationEditorModule::GetModule();
	AddMenuExtender(ItemizationEditor.GetMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));
	AddToolbarExtender(ItemizationEditor.GetToolBarExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));

	// Assume the default mode is always available
	SetCurrentMode(FItemDefinitionAppMode_Default::ModeId);

	RegenerateMenusAndToolbars();

	// Register the undo client
	GEditor->RegisterForUndo(this);
}

void FItemDefinitionApplication::AddItemDefinitionAppMode(
	UE::ItemizationEditor::FItemDefinitionAppModeInfo ModeArgs,
	TSharedRef<FItemDefinitionApplicationMode> ApplicationMode)
{
	CachedModeInfo.Add(ModeArgs.Identifier, ModeArgs);

	TabManager->AddLocalWorkspaceMenuItem(ApplicationMode->GetWorkspaceMenuCategory());

	AddApplicationMode(ModeArgs.Identifier, MoveTemp(ApplicationMode));

	UE_LOG(LogTemp, Log, TEXT("Added Item Definition Application Mode: %s [%s]"),
		*ModeArgs.Label.ToString(), *ModeArgs.Identifier.ToString());
}

void FItemDefinitionApplication::RemoveItemDefinitionAppMode(FName ModeName)
{
	CachedModeInfo.Remove(ModeName);
	RemoveApplicationMode(ModeName);
}

TOptional<UE::ItemizationEditor::FItemDefinitionAppModeInfo> FItemDefinitionApplication::GetModeInfo(FName ModeName) const
{
	using namespace UE::ItemizationEditor;

	TOptional<FItemDefinitionAppModeInfo> ModeInfo;

	if (const FItemDefinitionAppModeInfo* FoundInfo = CachedModeInfo.Find(ModeName))
	{
		ModeInfo = *FoundInfo;
	}

	return ModeInfo;
}

TArray<FName> FItemDefinitionApplication::GetApplicationModes() const
{
	TArray<FName> ApplicationModeNames;
	CachedModeInfo.GenerateKeyArray(ApplicationModeNames);
	return ApplicationModeNames;
}

FName FItemDefinitionApplication::GetToolkitFName() const
{
	return UE::ItemizationEditor::Ids::AppId;
}

FText FItemDefinitionApplication::GetBaseToolkitName() const
{
	return NSLOCTEXT("ItemizationEditor", "AppLabel", "Item Definition Editor");
}

FString FItemDefinitionApplication::GetWorldCentricTabPrefix() const
{
	return GetBaseToolkitName().ToString();
}

FString FItemDefinitionApplication::GetDocumentationLink() const
{
	if (IsValid(ItemDefinition))
	{
		return ItemDefinition->GetDocumentationURL();
	}

	return FString();
}

void FItemDefinitionApplication::BrowseDocumentation_Execute() const
{
	if (GetDocumentationLink().IsEmpty())
	{
		return;
	}

	IDocumentation::Get()->Open(GetDocumentationLink(), FDocumentationSourceInfo(TEXT("help_menu_asset")));
}

FLinearColor FItemDefinitionApplication::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.f, 0.2f, 0.8f, 0.5f);
}

FText FItemDefinitionApplication::GetToolkitName() const
{
	if (IsValid(ItemDefinition) && !ItemDefinition->GetItemName().IsEmpty())
	{
		return ItemDefinition->GetItemName();
	}

	return IItemDefinitionApplication::GetToolkitName();
}

FText FItemDefinitionApplication::GetToolkitToolTipText() const
{
	FText ToolTipText = IItemDefinitionApplication::GetToolkitToolTipText();

	if (IsValid(ItemDefinition) && !ItemDefinition->GetItemDescription().IsEmpty())
	{
		ToolTipText = FText::Format(
			NSLOCTEXT("ItemizationEditor", "ItemDefinitionTooltip", "{0}\n\n{1}"),
			ToolTipText,
			ItemDefinition->GetItemDescription());
	}

	return ToolTipText;
}

void FItemDefinitionApplication::OnClose()
{
	if (HostedToolkit.IsValid())
	{
		UToolMenus::UnregisterOwner(&*HostedToolkit);
		HostedToolkit = nullptr;
	}
}

void FItemDefinitionApplication::AddApplicationMode(FName ModeName, TSharedRef<FApplicationMode> Mode)
{
	// Register tabs for a mode once it's registered so that other modes have access to that tab.
	FWorkflowCentricApplication::AddApplicationMode(ModeName, Mode);
}

void FItemDefinitionApplication::SetCurrentMode(FName NewMode)
{
	FWorkflowCentricApplication::SetCurrentMode(NewMode);

	AppModeChangedDelegate.Broadcast(NewMode);
}

void FItemDefinitionApplication::PostInitAssetEditor()
{
	IItemDefinitionApplication::PostInitAssetEditor();

	IModularFeatures::Get().OnModularFeatureRegistered().AddSP(this, &ThisClass::OnModularFeatureRegistered);

	ModeUILayer = MakeShared<FItemDefinitionEditorModeUILayer>(ToolkitHost.Pin().Get());
	ModeUILayer->SetModeMenuCategory(WorkspaceMenuCategory);

	FName ToolMenuToolbarName;
	GetToolMenuToolbarName(ToolMenuToolbarName);
	ModeUILayer->SetSecondaryModeToolbarName(ToolMenuToolbarName);
	ToolkitCommands->Append(ModeUILayer->GetModeCommands());

	if (UContextObjectStore* ContextStore = EditorModeManager->GetInteractiveToolsContext()->ContextObjectStore)
	{
		UItemDefinitionEditorContext* ItemDefinitionContext = ContextStore->FindContext<UItemDefinitionEditorContext>();
		if (!IsValid(ItemDefinitionContext))
		{
			ItemDefinitionContext = NewObject<UItemDefinitionEditorContext>();
			ItemDefinitionContext->EditorHostInterface = EditorHost;
			ContextStore->AddContextObject(ItemDefinitionContext);
		}
	}
}

void FItemDefinitionApplication::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{
	ModeUILayer->OnToolkitHostingStarted(Toolkit);
	HostedToolkit = Toolkit;
}

void FItemDefinitionApplication::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{
	ModeUILayer->OnToolkitHostingFinished(Toolkit);
	HostedToolkit = nullptr;
}

void FItemDefinitionApplication::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (IsValid(ItemDefinition))
	{
		Collector.AddReferencedObject(ItemDefinition);
	}
}

void FItemDefinitionApplication::NotifyPreChange(FProperty* PropertyAboutToChange)
{
}

void FItemDefinitionApplication::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
}

void FItemDefinitionApplication::OnModularFeatureRegistered(const FName& Type, class IModularFeature* ModularFeature)
{
	if (Type != IItemDefinitionApplicationModeFactory::ModularFeatureName)
	{
		return;
	}

	IItemDefinitionApplicationModeFactory* Factory = static_cast<IItemDefinitionApplicationModeFactory*>(ModularFeature);

	TSharedRef<FItemDefinitionApplicationMode> AppMode = Factory->CreateItemDefinitionApplicationMode(SharedThis<ThisClass>(this));
	AddItemDefinitionAppMode(AppMode->GetModeInfo(), MoveTemp(AppMode));
}

void FItemDefinitionApplication::CreateInternalWidgets()
{
	FPropertyEditorModule& PropEdMod = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedPtr<ThisClass> ThisPtr(SharedThis(this));

	FDetailsViewArgs ViewArgs;
	{
		ViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		ViewArgs.NotifyHook = this;
		ViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Show;
		ViewArgs.bAllowFavoriteSystem = false;
		ViewArgs.bShowPropertyMatrixButton = false;
	}

	if (!AssetDetailsView.IsValid())
	{
		AssetDetailsView = PropEdMod.CreateDetailView(ViewArgs);
		AssetDetailsView->SetObject(nullptr);
		AssetDetailsView->ClearSearch();

		AssetDetailsView->RegisterInstancedCustomPropertyLayout(
			UItemDefinitionBase::StaticClass(),
			FOnGetDetailCustomizationInstance()/*::CreateRaw(this, &FItemizationEditorApplication::GetDetailsCustomization)*/);
	}

	if (!SelectionDetailsView.IsValid())
	{
		SelectionDetailsView = PropEdMod.CreateDetailView(ViewArgs);
		SelectionDetailsView->SetObject(nullptr);
		SelectionDetailsView->ClearSearch();

		SelectionDetailsView->RegisterInstancedCustomPropertyLayout(
			UItemDefinitionBase::StaticClass(),
			FOnGetDetailCustomizationInstance()/*::CreateRaw(this, &FItemizationEditorApplication::GetDetailsCustomization)*/);
	}
}

TSharedRef<SWidget> FItemDefinitionApplication::SpawnTab_AssetDetails(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == UE::ItemizationEditor::Ids::TabId_Details);

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.VAlign(VAlign_Fill)
		[
			AssetDetailsView.ToSharedRef()
		];
}

TSharedRef<SWidget> FItemDefinitionApplication::SpawnTab_ComponentData(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == UE::ItemizationEditor::Ids::TabId_ComponentData);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.TabColorScale(GetTabColorScale())
		[
			SAssignNew(ItemComponentDataView, SItemComponentDataView, ItemDefinitionViewModel.ToSharedRef())
		];

	return SpawnedTab;
}

TSharedRef<SWidget> FItemDefinitionApplication::SpawnTab_SelectionDetails(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == UE::ItemizationEditor::Ids::TabId_SelectionDetails);

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.VAlign(VAlign_Fill)
		[
			SelectionDetailsView.ToSharedRef()
		];
}

TSharedRef<SWidget> FItemDefinitionApplication::SpawnTab_DeveloperDetails(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == UE::ItemizationEditor::Ids::TabId_Developer);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.TabRole(ETabRole::PanelTab);

	return SpawnedTab;
}

void FItemDefinitionApplication::RegisterToolbar() const
{
	const UToolMenus* ToolMenus = UToolMenus::Get();
	FName ParentName;
	const FName MenuName = GetToolMenuToolbarName(ParentName);
	if (!ToolMenus->IsMenuRegistered(MenuName))
	{
		UToolMenus::Get()->RegisterMenu(MenuName, ParentName, EMultiBoxType::ToolBar);
	}
}

void FItemDefinitionApplication::RegisterMenus() const
{
	UToolMenus* ToolMenus = UToolMenus::Get();

	/*// ToolBar -> File
	{
		const FName MenuName = *(GetToolMenuName().ToString() + TEXT(".File"));
		if (!ToolMenus->IsMenuRegistered(MenuName))
		{
			const FName ParentMenuName = TEXT("MainFrame.MainMenu.File");
			ToolMenus->RegisterMenu(MenuName, ParentMenuName, EMultiBoxType::ToolBar);
			{
				UToolMenu* Menu = ToolMenus->RegisterMenu(MenuName, ParentMenuName);
				FToolMenuSection& Section = Menu->AddSection("ItemDefinition", LOCTEXT("ItemDefinitionHeading", "Item Definition"));

				const FToolMenuInsert InsertPosition("FileLoadAndSave", EToolMenuInsertType::After);
				Section.InsertPosition = InsertPosition;
			}
		}
	}

	// ToolBar -> Edit
	{
		const FName MenuName = *(GetToolMenuName().ToString() + TEXT(".Edit"));
		if (!ToolMenus->IsMenuRegistered(MenuName))
		{
			const FName ParentMenuName = TEXT("MainFrame.MainMenu.Edit");
			ToolMenus->RegisterMenu(MenuName, ParentMenuName, EMultiBoxType::ToolBar);
			{
				UToolMenu* Menu = ToolMenus->RegisterMenu(MenuName, ParentMenuName);
				FToolMenuSection& Section = Menu->AddSection("ItemDefinition", LOCTEXT("ItemDefinitionHeading", "Item Definition"));

				const FToolMenuInsert InsertPosition("Configuration", EToolMenuInsertType::After);
				Section.InsertPosition = InsertPosition;
			}
		}
	}*/
}

#undef LOCTEXT_NAMESPACE
