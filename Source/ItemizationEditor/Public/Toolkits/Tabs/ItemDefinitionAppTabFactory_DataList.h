// Author: Tom Werner (MajorT), 2025 November

#pragma once
#include "ItemDefinitionAppTabFactory.h"

class FItemDefinitionAppTabFactory_DataList : public FItemDefinitionAppTabFactory
{
public:
	FItemDefinitionAppTabFactory_DataList(const TSharedPtr<IItemDefinitionApp>& InApp);

protected:
	//~ Begin FWorkflowTabFactory Interface
	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;
	//~ End FWorkflowTabFactory Interface
};
