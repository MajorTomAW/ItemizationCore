// Author: Tom Werner (MajorT), 2025

#pragma once

#include "WorkflowOrientedApp/WorkflowTabFactory.h"

class IItemDefinitionApplication;

class ITEMIZATIONEDITOR_API FItemDefinitionEditorTabFactory : public FWorkflowTabFactory
{
public:
	FItemDefinitionEditorTabFactory(FName InTabId, const TSharedPtr<IItemDefinitionApplication>& InApp);

protected:
	inline TSharedPtr<IItemDefinitionApplication> GetApp() const
	{
		return WeakApp.Pin();
	}

	template <class ApplicationType = IItemDefinitionApplication>
	inline TSharedPtr<ApplicationType> GetAppAs() const
	{
		return StaticCastSharedPtr<ApplicationType>(WeakApp.Pin());
	}

private:
	TWeakPtr<IItemDefinitionApplication> WeakApp;
};
