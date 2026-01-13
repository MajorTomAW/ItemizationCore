// Author: Tom Werner (MajorT), 2025 November


#include "ItemDefinitionAppModeFactory_DataList.h"

#include "Toolkits/Modes/ItemDefinitionAppMode_DataList.h"

TSharedRef<FItemDefinitionAppMode> FItemDefinitionAppModeFactory_DataList::CreateItemDefinitionAppMode(
	TSharedPtr<IItemDefinitionApp> InApp)
{
	return MakeShared<FItemDefinitionAppMode_DataList>(InApp);
}
