// Author: Tom Werner (MajorT), 2025


#include "ItemDefinitionAppModeFactory_DataList.h"

#include "Toolkits/Modes/ItemDefinitionAppMode_DataList.h"

TSharedRef<FItemDefinitionApplicationMode> FItemDefinitionAppModeFactory_DataList::CreateItemDefinitionApplicationMode(
	TSharedPtr<IItemDefinitionApplication> InApp)
{
	return MakeShared<FItemDefinitionAppMode_DataList>(InApp);
}
