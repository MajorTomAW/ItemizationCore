// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "Toolkits/IItemDefinitionApp.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

class IItemDefinitionAppModeFactory : public IModularFeature
{
public:
	virtual ~IItemDefinitionAppModeFactory() = default;

	/** Name of the modular feature. */
	static const FName ModularFeatureName;

	/** Instantiate an app mode so the item definition app can register it and display it in its layout selector. */
	virtual TSharedRef<class FItemDefinitionAppMode> CreateItemDefinitionAppMode(TSharedPtr<IItemDefinitionApp> InApp) = 0;
};

/** Base class for all application modes in the item definition app. */
class ITEMIZATIONEDITOR_API FItemDefinitionAppMode
	: public FApplicationMode
{
	using ThisClass = FItemDefinitionAppMode;

public:
	FItemDefinitionAppMode() = delete;
	FItemDefinitionAppMode(const UE::ItemizationEditor::FItemDefinitionAppModeInfo& InModeInfo, const TSharedPtr<IItemDefinitionApp>& InApp);

	/** Returns the cached mode info for this app mode. */
	const UE::ItemizationEditor::FItemDefinitionAppModeInfo& GetModeInfo()
	{
		return ModeInfo;
	}

	/** Returns the tab layout associated with this app mode. */
	TSharedPtr<FTabManager::FLayout> GetTabLayout() const
	{
		return TabLayout;
	}

	//~ Begin FApplicationMode Interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	virtual void PreDeactivateMode() override;
	virtual void PostActivateMode() override;
	//~ End FApplicationMode Interface

protected:
	void AddModesToolbar(TSharedPtr<FExtender> Extender);
	virtual void FillModesToolbar(FToolBarBuilder& ToolBarBuilder);

	template <class AppType = IItemDefinitionApp>
	TSharedPtr<AppType> GetApp() const
	{
		return StaticCastSharedPtr<AppType>(WeakApp.Pin());
	}

protected:
	/** Tab set to spawn once activated. */
	FWorkflowAllowedTabSet TabSet;

	/** Const arguments for the application mode. */
	UE::ItemizationEditor::FItemDefinitionAppModeInfo ModeInfo;

	/** The application that hosts this app mode. */
	TWeakPtr<IItemDefinitionApp> WeakApp;
};
