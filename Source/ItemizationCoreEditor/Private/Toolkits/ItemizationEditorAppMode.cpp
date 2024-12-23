// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/ItemizationEditorAppMode.h"

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
