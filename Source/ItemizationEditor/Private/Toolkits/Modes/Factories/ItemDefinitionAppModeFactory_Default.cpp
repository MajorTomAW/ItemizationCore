// Author: Tom Werner (MajorT), 2025 November


#include "ItemDefinitionAppModeFactory_Default.h"

#include "Toolkits/Modes/ItemDefinitionAppMode_Default.h"

TSharedRef<FItemDefinitionAppMode> FItemDefinitionAppModeFactory_Default::CreateItemDefinitionAppMode(
	TSharedPtr<IItemDefinitionApp> InApp)
{
	return MakeShared<FItemDefinitionAppMode_Default>(InApp);
}
