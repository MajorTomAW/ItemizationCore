// Author: Tom Werner (MajorT), 2025

#pragma once
#include "ItemDefinitionEditorTabFactory.h"

class FItemDefinitionEditorTabFactory_Details : public FItemDefinitionEditorTabFactory
{
public:
	FItemDefinitionEditorTabFactory_Details(const TSharedPtr<IItemDefinitionApplication>& InApp);

protected:
	//~ Begin FWorkflowTabFactory Interface
	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;
	//~ End FWorkflowTabFactory Interface
};