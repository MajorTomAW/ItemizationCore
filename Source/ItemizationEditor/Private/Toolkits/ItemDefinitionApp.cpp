// Author: Tom Werner (MajorT), 2025 November


#include "ItemDefinitionApp.h"

#include "IDocumentation.h"
#include "IStructureDetailsView.h"
#include "ItemDefinitionEditingSubsystem.h"
#include "ItemizationEditorModule.h"
#include "StandaloneItemDefinitionAppHost.h"
#include "Customization/ItemDefinitionAppDetailsViewCustomization.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Items/ItemDefinitionBase.h"
#include "Modes/ItemDefinitionAppMode_Default.h"
#include "Slate/Toolkits/SItemDataListView.h"
#include "Slate/Toolkits/SItemDefinitionAppViewport.h"
#include "Toolkits/ItemDefinitionViewModel.h"
#include "Toolkits/Modes/ItemDefinitionAppMode.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionApp"

FItemDefinitionApp::FItemDefinitionApp()
	: DataListCommands(new FUICommandList())
{
}

void FItemDefinitionApp::InitApp(
	const EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost,
	UItemDefinitionBase* InItemDefinition)
{
	ItemDefinition = InItemDefinition;
	check(ItemDefinition)

	UItemDefinitionEditingSubsystem* ItemEdSub = UItemDefinitionEditingSubsystem::Get();
	check(ItemEdSub)

	AppHost = MakeShared<FStandaloneItemDefinitionAppHost>();
	AppHost->Init(StaticCastSharedRef<FItemDefinitionApp>(AsShared()));

	ItemViewModel = ItemEdSub->FindOrAddViewModel(InItemDefinition);

	TSharedPtr<FItemDefinitionApp> ThisPtr(SharedThis(this));

	// Init the editor things
	CreateEditorModeManager();
	CreateInternalWidgets();

	const TSharedRef<FTabManager::FLayout> DummyLayout = FTabManager::NewLayout("NullLayout")->AddArea(FTabManager::NewPrimaryArea());
	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar = true;

	// Initialize the asset editor with a placeholder layout, as the actual layouts will be provided by the application modes.
	FAssetEditorToolkit::InitAssetEditor(
		Mode,
		InitToolkitHost,
		UE::ItemizationEditor::Ids::AppId,
		DummyLayout,
		bCreateDefaultStandaloneMenu,
		bCreateDefaultToolbar,
		ItemDefinition);

	// Register application modes using the modular feature system.
	for (IItemDefinitionAppModeFactory* Factory : IModularFeatures::Get()
		.GetModularFeatureImplementations<IItemDefinitionAppModeFactory>(IItemDefinitionAppModeFactory::ModularFeatureName))
	{
		TSharedRef<FItemDefinitionAppMode> ApplicationMode =
			Factory->CreateItemDefinitionAppMode(SharedThis<ThisClass>(this));

		AddItemDefinitionAppMode(ApplicationMode->GetModeInfo(), MoveTemp(ApplicationMode));
	}

	FLayoutExtender LayoutExtender;
	IItemizationEditorModule& ItemizationEditor = IItemizationEditorModule::GetModule();
	AddMenuExtender(ItemizationEditor.GetMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));
	AddToolbarExtender(ItemizationEditor.GetToolBarExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));

	// Assume the default mode is always available
	SetCurrentMode(FItemDefinitionAppMode_Default::ModeId);

	RegenerateMenusAndToolbars();
}

UItemDefinitionBase* FItemDefinitionApp::GetItemDefinition() const
{
	return ItemDefinition;
}

TArray<FName> FItemDefinitionApp::GetApplicationModes() const
{
	TArray<FName> ModeIds;
	CachedModeInfo.GenerateKeyArray(ModeIds);
	return ModeIds;
}

void FItemDefinitionApp::AddItemDefinitionAppMode(
	UE::ItemizationEditor::FItemDefinitionAppModeInfo ModeArgs,
	TSharedRef<FItemDefinitionAppMode> ApplicationMode)
{
	CachedModeInfo.Add(ModeArgs.Id, ModeArgs);
	TabManager->AddLocalWorkspaceMenuItem(ApplicationMode->GetWorkspaceMenuCategory());

	AddApplicationMode(ModeArgs.Id, MoveTemp(ApplicationMode));
}

void FItemDefinitionApp::RemoveItemDefinitionAppMode(FName ModeName)
{
	CachedModeInfo.Remove(ModeName);
	RemoveApplicationMode(ModeName);
}

TOptional<UE::ItemizationEditor::FItemDefinitionAppModeInfo> FItemDefinitionApp::GetModeInfo(FName ModeName) const
{
	using namespace UE::ItemizationEditor;

	TOptional<FItemDefinitionAppModeInfo> ModeInfo;
	if (const FItemDefinitionAppModeInfo* Info = CachedModeInfo.Find(ModeName))
	{
		ModeInfo = *Info;
	}

	return ModeInfo;
}

void FItemDefinitionApp::AddApplicationMode(FName ModeName, TSharedRef<FApplicationMode> Mode)
{
	// Register tabs for a mode once it's registered so that other modes have access to that tab.
	FWorkflowCentricApplication::AddApplicationMode(ModeName, Mode);
}

void FItemDefinitionApp::SetCurrentMode(FName NewMode)
{
	IItemDefinitionApp::SetCurrentMode(NewMode);

	AppModeChangedDelegate.Broadcast(NewMode);
}

void FItemDefinitionApp::NotifyPreChange(FProperty* PropertyAboutToChange)
{
	FNotifyHook::NotifyPreChange(PropertyAboutToChange);

	UE_LOG(LogTemp, Warning, TEXT("PreChange: %s"), *PropertyAboutToChange->GetName())

	if (IsValid(ItemDefinition))
	{
		ItemDefinition->PreEditChange(PropertyAboutToChange);
	}
}

void FItemDefinitionApp::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent,
	FProperty* PropertyThatChanged)
{
	FNotifyHook::NotifyPostChange(PropertyChangedEvent, PropertyThatChanged);

	UE_LOG(LogTemp, Warning, TEXT("PostChange: %s"), *PropertyThatChanged->GetName())

	if (IsValid(ItemDefinition))
	{
		ItemDefinition->PostEditChangeProperty(const_cast<FPropertyChangedEvent&>(PropertyChangedEvent));
		ItemDefinition->MarkPackageDirty();
	}
}

void FItemDefinitionApp::PostInitAssetEditor()
{
	IItemDefinitionApp::PostInitAssetEditor();

	IModularFeatures::Get().OnModularFeatureRegistered().AddSP(this, &ThisClass::OnModularFeatureRegistered);
}

void FItemDefinitionApp::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{
	HostedToolkit = nullptr;
}

void FItemDefinitionApp::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{
	HostedToolkit = Toolkit;
}

FName FItemDefinitionApp::GetToolkitFName() const
{
	return UE::ItemizationEditor::Ids::AppId;
}

FText FItemDefinitionApp::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Item Definition");
}

FString FItemDefinitionApp::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("TabPrefix", "Item Definition").ToString();
}

FString FItemDefinitionApp::GetDocumentationLink() const
{
	if (IsValid(ItemDefinition))
	{
		return ItemDefinition->GetDocumentationURL();
	}

	return IItemDefinitionApp::GetDocumentationLink();
}

void FItemDefinitionApp::BrowseDocumentation_Execute() const
{
	if (GetDocumentationLink().IsEmpty())
	{
		return;
	}

	IDocumentation::Get()->Open(GetDocumentationLink(), FDocumentationSourceInfo(TEXT("help_menu_asset")));
}

FText FItemDefinitionApp::GetToolkitName() const
{
	return IItemDefinitionApp::GetToolkitName();
}

FText FItemDefinitionApp::GetToolkitToolTipText() const
{
	return IItemDefinitionApp::GetToolkitToolTipText();
}

FLinearColor FItemDefinitionApp::GetWorldCentricTabColorScale() const
{
	return FItemizationEditorStyle::Get().GetColor("Colors.TabColorScale");
}

void FItemDefinitionApp::OnClose()
{
	if (HostedToolkit.IsValid())
	{
		UToolMenus::UnregisterOwner(&(*HostedToolkit));
		HostedToolkit = nullptr;
	}
}

void FItemDefinitionApp::PostUndo(bool bSuccess)
{
	if (bSuccess && ItemDefinition)
	{
		/*if (TSharedPtr<IStructureDetailsView> View = GetDataListDetailsView())
		{
			View->SetStructureData(nullptr);
		}*/

		if (UItemDefinitionEditingSubsystem* ItemEdSub = UItemDefinitionEditingSubsystem::Get())
		{
			TSharedPtr<FItemDefinitionViewModel> VM = ItemEdSub->FindOrAddViewModel(ItemDefinition);

			FPropertyChangedEvent ChangedEvent(nullptr, EPropertyChangeType::ValueSet);
			TSharedPtr<FItemDataUIInfo> DummyInfo;
			VM->GetOnItemDataChanged().Broadcast(DummyInfo, ChangedEvent);
		}
	}
}

void FItemDefinitionApp::PostRedo(bool bSuccess)
{
	if (bSuccess && ItemDefinition)
	{
		/*if (TSharedPtr<IStructureDetailsView> View = GetDataListDetailsView())
		{
			View->SetStructureData(nullptr);
		}*/

		if (UItemDefinitionEditingSubsystem* ItemEdSub = UItemDefinitionEditingSubsystem::Get())
		{
			TSharedPtr<FItemDefinitionViewModel> VM = ItemEdSub->FindOrAddViewModel(ItemDefinition);

			FPropertyChangedEvent ChangedEvent(nullptr, EPropertyChangeType::ValueSet);
			TSharedPtr<FItemDataUIInfo> DummyInfo;
			VM->GetOnItemDataChanged().Broadcast(DummyInfo, ChangedEvent);
		}
	}
}

void FItemDefinitionApp::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (ItemDefinition != nullptr)
	{
		Collector.AddReferencedObject(ItemDefinition);
	}
}

void FItemDefinitionApp::OnModularFeatureRegistered(const FName& Type, class IModularFeature* ModularFeature)
{
	if (Type != IItemDefinitionAppModeFactory::ModularFeatureName)
	{
		return;
	}

	IItemDefinitionAppModeFactory* Factory = static_cast<IItemDefinitionAppModeFactory*>(ModularFeature);

	TSharedRef<FItemDefinitionAppMode> AppMode = Factory->CreateItemDefinitionAppMode(SharedThis<ThisClass>(this));
	AddItemDefinitionAppMode(AppMode->GetModeInfo(), MoveTemp(AppMode));
}

void FItemDefinitionApp::OnDataListFinishedChangingProperty(const FPropertyChangedEvent& ChangedEvent)
{
	if (IsValid(ItemDefinition))
	{
		if (UItemDefinitionEditingSubsystem* ItemEdSub = UItemDefinitionEditingSubsystem::Get())
		{
			TSharedPtr<FItemDefinitionViewModel> VM = ItemEdSub->FindOrAddViewModel(ItemDefinition);

			TSharedPtr<FItemDataUIInfo> DummyInfo;
			VM->GetOnItemDataChanged().Broadcast(DummyInfo, ChangedEvent);
		}

		UE_LOG(LogTemp, Warning, TEXT("FinishedChangingProperty: %s"), *ChangedEvent.Property->GetName())
	}
}

void FItemDefinitionApp::CreateInternalWidgets()
{
	FPropertyEditorModule& PropEdMod = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs ViewArgs;
	{
		ViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
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
			FOnGetDetailCustomizationInstance::CreateRaw(this, &FItemDefinitionApp::GetAssetDetailsCustomization));
	}
	if (!SettingsDetailsView.IsValid())
	{
		SettingsDetailsView = PropEdMod.CreateDetailView(ViewArgs);
		SettingsDetailsView->SetObject(nullptr);
		SettingsDetailsView->ClearSearch();

		SettingsDetailsView->RegisterInstancedCustomPropertyLayout(
			UItemDefinitionBase::StaticClass(),
			FOnGetDetailCustomizationInstance::CreateRaw(this, &FItemDefinitionApp::GetSettingsDetailsCustomization));
	}

	if (!DataListDetailsView.IsValid())
	{
		ViewArgs.NotifyHook = this;

		FStructureDetailsViewArgs StructViewArgs;
		StructViewArgs.bShowAssets = true;
		StructViewArgs.bShowClasses = true;
		StructViewArgs.bShowInterfaces = true;
		StructViewArgs.bShowObjects = true;

		DataListDetailsView = PropEdMod.CreateStructureDetailView(ViewArgs, StructViewArgs, nullptr);
		//DataListDetailsView->GetOnFinishedChangingPropertiesDelegate().AddRaw(this, &ThisClass::OnDataListFinishedChangingProperty);
	}
}

TSharedRef<SWidget> FItemDefinitionApp::SpawnTab_AssetDetails(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == UE::ItemizationEditor::Ids::TabId_AssetDetails)

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.VAlign(VAlign_Fill)
		[
			AssetDetailsView.ToSharedRef()
		];
}

TSharedRef<SWidget> FItemDefinitionApp::SpawnTab_Settings(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == UE::ItemizationEditor::Ids::TabId_Settings)

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.VAlign(VAlign_Fill)
		[
			SettingsDetailsView.ToSharedRef()
		];
}

TSharedRef<SWidget> FItemDefinitionApp::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == UE::ItemizationEditor::Ids::TabId_Viewport)

	if (!Viewport.IsValid())
	{
		Viewport = SNew(SItemDefinitionAppViewport);
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.VAlign(VAlign_Fill)
		[
			Viewport.ToSharedRef()
		];
}

TSharedRef<SWidget> FItemDefinitionApp::SpawnTab_DataList(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == UE::ItemizationEditor::Ids::TabId_DataList)

	if (!DataListView.IsValid())
	{
		DataListView = SNew(SItemDataListView, ItemViewModel.ToSharedRef(), DataListCommands);
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.VAlign(VAlign_Fill)
		[
			DataListView.ToSharedRef()
		];
}

TSharedRef<SWidget> FItemDefinitionApp::SpawnTab_DataListDetails(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == UE::ItemizationEditor::Ids::TabId_DataListDetails)

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.VAlign(VAlign_Fill)
		[
			DataListDetailsView->GetWidget().ToSharedRef()
		];
}

TSharedRef<IDetailCustomization> FItemDefinitionApp::GetAssetDetailsCustomization()
{
	using namespace UE::ItemizationEditor;

	return FAssetDetailsViewCustomization::MakeInstance(
		FAssetDetailsViewArgs::AssetDetailsView(),
		SharedThis(this));
}

TSharedRef<IDetailCustomization> FItemDefinitionApp::GetSettingsDetailsCustomization()
{
	using namespace UE::ItemizationEditor;

	return FAssetDetailsViewCustomization::MakeInstance(
		FAssetDetailsViewArgs::SettingsDetailsView(),
		SharedThis(this));
}

#undef LOCTEXT_NAMESPACE
