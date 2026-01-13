// Author: Tom Werner (MajorT), 2025 November


#include "Toolkits/Tabs/ItemDefinitionAppTabFactory.h"
#include "Toolkits/IItemDefinitionApp.h"

FItemDefinitionAppTabFactory::FItemDefinitionAppTabFactory(
	FName InTabId,
	const TSharedPtr<IItemDefinitionApp>& InApp)
		: FWorkflowTabFactory(InTabId, InApp)
{
	bIsSingleton = true;
	WeakApp = InApp;
	TabRole = PanelTab;
}
