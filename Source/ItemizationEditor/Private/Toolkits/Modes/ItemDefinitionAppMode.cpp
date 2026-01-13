// Author: Tom Werner (MajorT), 2025 November

#include "Toolkits/Modes/ItemDefinitionAppMode.h"

#include "Slate/Widgets/SItemDefinitionAppAssetStatus.h"
#include "Slate/Widgets/SItemDefinitionAppAssetType.h"
#include "Slate/Widgets/SItemDefinitionAppModeSwitcher.h"

const FName IItemDefinitionAppModeFactory::ModularFeatureName = "ItemDefinitionAppModeFactory";

FItemDefinitionAppMode::FItemDefinitionAppMode(
	const UE::ItemizationEditor::FItemDefinitionAppModeInfo& InModeInfo,
	const TSharedPtr<IItemDefinitionApp>& InApp)
		: FApplicationMode(InModeInfo.Id)
{
	ModeInfo = InModeInfo;
	WeakApp = InApp;
}

void FItemDefinitionAppMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	const TSharedPtr<IItemDefinitionApp> AppPtr = GetApp();
	AppPtr->PushTabFactories(TabSet);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FItemDefinitionAppMode::PreDeactivateMode()
{
	FApplicationMode::PreDeactivateMode();
}

void FItemDefinitionAppMode::PostActivateMode()
{
	FApplicationMode::PostActivateMode();
}

void FItemDefinitionAppMode::AddModesToolbar(TSharedPtr<FExtender> Extender)
{
	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetApp()->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateRaw(this, &ThisClass::FillModesToolbar));
}

void FItemDefinitionAppMode::FillModesToolbar(FToolBarBuilder& ToolBarBuilder)
{
	TSharedPtr<IItemDefinitionApp> AppPtr = GetApp();

	// Asset Status
	ToolBarBuilder.BeginSection("AssetStatus");
	ToolBarBuilder.AddWidget(SNew(SItemDefinitionAppAssetStatus, AppPtr));
	ToolBarBuilder.EndSection();

	// Asset Info
	ToolBarBuilder.BeginSection("AssetInfo");
	ToolBarBuilder.AddWidget(SNew(SItemDefinitionAppAssetType, AppPtr));
	ToolBarBuilder.EndSection();

	// Modes
	ToolBarBuilder.BeginSection("Modes");
	ToolBarBuilder.AddWidget(SNew(SItemDefinitionAppModeSwitcher, AppPtr, ToolBarBuilder).bUseDropdown(false));
	ToolBarBuilder.EndSection();
}
