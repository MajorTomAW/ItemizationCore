// Author: Tom Werner (MajorT), 2025 November

#pragma once
#include "Toolkits/Modes/ItemDefinitionAppMode.h"

class FItemDefinitionAppModeFactory_Default : public IItemDefinitionAppModeFactory
{
public:
	virtual TSharedRef<FItemDefinitionAppMode> CreateItemDefinitionAppMode(TSharedPtr<IItemDefinitionApp> InApp) override;
};
