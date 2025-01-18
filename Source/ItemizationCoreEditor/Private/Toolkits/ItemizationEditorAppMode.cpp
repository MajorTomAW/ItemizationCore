// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/ItemizationEditorAppMode.h"

#include "ItemizationCoreEditorHelpers.h"
#include "Toolkits/ItemizationEditorApplication.h"


FItemizationEditorAppMode::FItemizationEditorAppMode(
	TSharedPtr<FItemizationEditorApplication> InApp, const IItemizationCoreEditorModule::FItemizationAppModeArgs& InArgs)
		: FApplicationMode(InArgs.ModeId)
{
	Args = InArgs;
	WeakApp = InApp;
}

void FItemizationEditorAppMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	TSharedPtr<FItemizationEditorApplication> App = GetApp();
	App->PushTabFactories(TabSet);
	
	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FItemizationEditorAppMode::PreDeactivateMode()
{
	FApplicationMode::PreDeactivateMode();
}

void FItemizationEditorAppMode::PostActivateMode()
{
	FApplicationMode::PostActivateMode();
}

bool FItemizationEditorAppMode::CanActivateMode() const
{
	return true;
}

bool FItemizationEditorAppMode::CanShowMode() const
{
	TSharedPtr<FItemizationEditorApplication> App = GetApp();
	const TSharedPtr<FItemizationEditorAssetConfig> Config = App->GetAssetConfig();
	if (Config.IsValid())
	{
		return Config->CanShowAppMode(Args.ModeId);
	}
	
	
	return true;
}
