// Author: Tom Werner (MajorT), 2025 November

#pragma once
#include "ItemDefinitionAppTabFactory.h"

class FItemDefinitionAppTabFactory_Settings : public FItemDefinitionAppTabFactory
{
public:
	FItemDefinitionAppTabFactory_Settings(const TSharedPtr<IItemDefinitionApp>& InApp);

protected:
	//~ Begin FWorkflowTabFactory Interface
	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;
	//~ End FWorkflowTabFactory Interface
};
