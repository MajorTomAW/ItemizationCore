// Author: Tom Werner (MajorT), 2025


#include "ItemDefinitionAppModeFactory_Default.h"

#include "Toolkits/Modes/ItemDefinitionAppMode_Default.h"

TSharedRef<FItemDefinitionApplicationMode> FItemDefinitionAppModeFactory_Default::CreateItemDefinitionApplicationMode(
	TSharedPtr<IItemDefinitionApplication> InApp)
{
	return MakeShared<FItemDefinitionAppMode_Default>(InApp);
}
