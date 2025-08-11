// Author: Tom Werner (MajorT), 2025


#include "Toolkits/ItemDefinitionApplicationMode.h"

#include "Slate/Widgets/SItemDefinitionEditorAssetStatus.h"
#include "Slate/Widgets/SItemDefinitionEditorAssetType.h"
#include "Slate/Widgets/SItemDefinitionEditorModeSwitcher.h"

const FName IItemDefinitionApplicationModeFactory::ModularFeatureName = "ItemDefinitionApplicationModeFactory";

FItemDefinitionApplicationMode::FItemDefinitionApplicationMode()
	: FApplicationMode("<INVALID MODE>")
{
}

FItemDefinitionApplicationMode::FItemDefinitionApplicationMode(
	const UE::ItemizationEditor::FItemDefinitionAppModeInfo& InModeInfo,
	const TSharedPtr<IItemDefinitionApplication>& InApp)
		: FApplicationMode(InModeInfo.Identifier)
{
	ModeInfo = InModeInfo;
	WeakApp = InApp;
}

void FItemDefinitionApplicationMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	const TSharedPtr<IItemDefinitionApplication> AppPtr = WeakApp.Pin();
	AppPtr->PushTabFactories(TabSet);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FItemDefinitionApplicationMode::PreDeactivateMode()
{
	FApplicationMode::PreDeactivateMode();
}

void FItemDefinitionApplicationMode::PostActivateMode()
{
	FApplicationMode::PostActivateMode();
}

void FItemDefinitionApplicationMode::AddModesToolbar(TSharedPtr<FExtender> Extender)
{
	Extender->AddToolBarExtension("Asset", EExtensionHook::After, WeakApp.Pin()->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateRaw(this, &ThisClass::FillModesToolbar));
}

void FItemDefinitionApplicationMode::FillModesToolbar(FToolBarBuilder& ToolBarBuilder)
{
	TSharedPtr<IItemDefinitionApplication> AppPtr = WeakApp.Pin();

	// Asset Status
	ToolBarBuilder.BeginSection("AssetStatus");
	ToolBarBuilder.AddWidget(SNew(SItemDefinitionEditorAssetStatus, AppPtr));
	ToolBarBuilder.EndSection();

	// Asset Info
	ToolBarBuilder.BeginSection("AssetInfo");
	ToolBarBuilder.AddWidget(SNew(SItemDefinitionEditorAssetType, AppPtr));
	ToolBarBuilder.EndSection();

	// Mode Switcher
	ToolBarBuilder.BeginSection("Modes");
	ToolBarBuilder.AddWidget(SNew(SItemDefinitionEditorModeSwitcher, AppPtr, ToolBarBuilder).bUseDropdown(false));
	ToolBarBuilder.EndSection();
}
