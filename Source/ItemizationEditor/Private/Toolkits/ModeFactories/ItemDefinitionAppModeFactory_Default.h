// Author: Tom Werner (MajorT), 2025

#pragma once
#include "Toolkits/ItemDefinitionApplicationMode.h"

class FItemDefinitionAppModeFactory_Default : public IItemDefinitionApplicationModeFactory
{
public:
	virtual TSharedRef<class FItemDefinitionApplicationMode> CreateItemDefinitionApplicationMode(TSharedPtr<IItemDefinitionApplication> InApp) override;
};