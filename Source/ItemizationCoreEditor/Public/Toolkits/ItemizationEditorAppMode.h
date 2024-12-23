// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemizationCoreEditor.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

class FItemizationEditorApplication;

/**
 * Default application mode for the Itemization Editor 
 */
class ITEMIZATIONCOREEDITOR_API FItemizationEditorAppMode : public FApplicationMode
{
	friend class FItemizationEditorToolbar;
	
public:
	FItemizationEditorAppMode(TSharedPtr<FItemizationEditorApplication> InApp, const IItemizationCoreEditorModule::FItemizationAppModeArgs& InArgs);

	//~ Begin FApplicationMode Interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	virtual void PreDeactivateMode() override;
	virtual void PostActivateMode() override;
	//~ End FApplicationMode Interface

	virtual bool CanActivateMode() const { return true; }
	virtual bool CanShowMode() const { return true; }

protected:
	TSharedPtr<FItemizationEditorApplication> GetApp() const { return WeakApp.Pin(); }
	const IItemizationCoreEditorModule::FItemizationAppModeArgs& GetArgs() const { return Args; }

	FWorkflowAllowedTabSet TabSet;

private:
	IItemizationCoreEditorModule::FItemizationAppModeArgs Args;
	TWeakPtr<FItemizationEditorApplication> WeakApp;
};
