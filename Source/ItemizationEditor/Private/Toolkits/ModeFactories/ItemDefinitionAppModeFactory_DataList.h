// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Toolkits/ItemDefinitionApplicationMode.h"

class FItemDefinitionAppModeFactory_DataList : public IItemDefinitionApplicationModeFactory
{
public:
	virtual TSharedRef<FItemDefinitionApplicationMode> CreateItemDefinitionApplicationMode(TSharedPtr<IItemDefinitionApplication> InApp) override;
};