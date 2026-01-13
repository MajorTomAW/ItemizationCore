// Author: Tom Werner (MajorT), 2025 November

#pragma once
#include "WorkflowOrientedApp/WorkflowTabFactory.h"

class IItemDefinitionApp;

class ITEMIZATIONEDITOR_API FItemDefinitionAppTabFactory
	: public FWorkflowTabFactory
{
public:
	FItemDefinitionAppTabFactory(FName InTabId, const TSharedPtr<IItemDefinitionApp>& InApp);

protected:
	template <class AppType = IItemDefinitionApp>
	inline TSharedPtr<AppType> GetApp() const
	{
		return StaticCastSharedPtr<AppType>(WeakApp.Pin());
	}

private:
	TWeakPtr<IItemDefinitionApp> WeakApp;
};
