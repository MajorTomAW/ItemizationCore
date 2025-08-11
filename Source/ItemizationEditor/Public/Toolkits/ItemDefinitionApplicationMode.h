// Author: Tom Werner (MajorT), 2025

#pragma once

#include "IItemDefinitionApplication.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

class IItemDefinitionApplication;

class ITEMIZATIONEDITOR_API IItemDefinitionApplicationModeFactory : public IModularFeature
{
public:
	virtual ~IItemDefinitionApplicationModeFactory() = default;
	
	/** Name of the modular feature. */
	static const FName ModularFeatureName;

	/** Instantiate an application mode so the Itemization Editor can register it and display it in its Layout selector. */
	virtual TSharedRef<class FItemDefinitionApplicationMode> CreateItemDefinitionApplicationMode(TSharedPtr<IItemDefinitionApplication> InApp) = 0;
};

class ITEMIZATIONEDITOR_API FItemDefinitionApplicationMode : public FApplicationMode
{
	using ThisClass = FItemDefinitionApplicationMode;
public:
	FItemDefinitionApplicationMode();
	FItemDefinitionApplicationMode(const UE::ItemizationEditor::FItemDefinitionAppModeInfo& InModeInfo, const TSharedPtr<IItemDefinitionApplication>& InApp);

	UE::ItemizationEditor::FItemDefinitionAppModeInfo GetModeInfo() const
	{
		return ModeInfo;
	}

	TSharedPtr<FTabManager::FLayout> GetTabLayout() const
	{
		return TabLayout;
	}

	/** Returns the mode icon brush from the mode info. */
	FSlateIcon GetModeIcon() const
	{
		return ModeInfo.Icon;
	}

	//~ Begin FApplicationMode Interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	virtual void PreDeactivateMode() override;
	virtual void PostActivateMode() override;
	//~ End FApplicationMode Interface

protected:
	void AddModesToolbar(TSharedPtr<FExtender> Extender);
	virtual void FillModesToolbar(FToolBarBuilder& ToolBarBuilder);

	TSharedPtr<IItemDefinitionApplication> GetApp() const
	{
		return WeakApp.Pin();
	}

	template <class ApplicationType = IItemDefinitionApplication>
	TSharedPtr<ApplicationType> GetAppAs() const
	{
		return StaticCastSharedPtr<ApplicationType>(WeakApp.Pin());
	}

protected:
	/** Tab set to spawn once activated. */
	FWorkflowAllowedTabSet TabSet;
	
	/** Const arguments for the application mode. */
	UE::ItemizationEditor::FItemDefinitionAppModeInfo ModeInfo;

	/** The application that hosts this app mode. */
	TWeakPtr<IItemDefinitionApplication> WeakApp;
};
