// Author: Tom Werner (MajorT), 2025


#include "Toolkits/Factories/ItemDefinitionEditorTabFactory.h"
#include "Toolkits/IItemDefinitionApplication.h"

FItemDefinitionEditorTabFactory::FItemDefinitionEditorTabFactory(
	FName InTabId,
	const TSharedPtr<IItemDefinitionApplication>& InApp)
		: FWorkflowTabFactory(InTabId, InApp)
{
	bIsSingleton = true;
	WeakApp = InApp;
	TabRole = PanelTab;
}
