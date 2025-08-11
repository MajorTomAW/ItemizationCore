// Author: Tom Werner (MajorT), 2025


#include "ItemDefinitionEditorUILayer.h"

FItemDefinitionEditorModeUILayer::FItemDefinitionEditorModeUILayer(const IToolkitHost* InToolkitHost)
	: FAssetEditorModeUILayer(InToolkitHost)
{
}

void FItemDefinitionEditorModeUILayer::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{
	if (!Toolkit->IsAssetEditor())
	{
		FAssetEditorModeUILayer::OnToolkitHostingStarted(Toolkit);
		HostedToolkit = Toolkit;
		Toolkit->SetModeUILayer(SharedThis(this));
		Toolkit->RegisterTabSpawners(ToolkitHost->GetTabManager().ToSharedRef());
		RegisterModeTabSpawners();

		OnToolkitHostReadyForUI.Execute();

		// Set up an owner for the current scope so that we can cleanly clean up the toolbar extension on hosting finish
		UToolMenu* SecondaryModeToolbar = UToolMenus::Get()->ExtendMenu(GetSecondaryModeToolbarName());
		OnRegisterSecondaryModeToolbarExtension.ExecuteIfBound(SecondaryModeToolbar);
	}
}

void FItemDefinitionEditorModeUILayer::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{
	if (HostedToolkit.IsValid() && HostedToolkit.Pin() == Toolkit)
	{
		FAssetEditorModeUILayer::OnToolkitHostingFinished(Toolkit);
	}
}

TSharedPtr<FWorkspaceItem> FItemDefinitionEditorModeUILayer::GetModeMenuCategory() const
{
	check(MenuCategory);
	return MenuCategory;
}

void FItemDefinitionEditorModeUILayer::SetModeMenuCategory(const TSharedPtr<FWorkspaceItem>& MenuCategoryIn)
{
	MenuCategory = MenuCategoryIn;
}
