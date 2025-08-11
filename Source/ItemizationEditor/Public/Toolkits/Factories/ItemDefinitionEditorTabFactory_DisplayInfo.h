// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "ItemDefinitionEditorTabFactory.h"

class FItemDefinitionEditorTabFactory_DisplayInfo : public FItemDefinitionEditorTabFactory
{
public:
	FItemDefinitionEditorTabFactory_DisplayInfo(const TSharedPtr<IItemDefinitionApplication>& InApp);

protected:
	//~ Begin FWorkflowTabFactory Interface
	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;
	//~ End FWorkflowTabFactory Interface
};
