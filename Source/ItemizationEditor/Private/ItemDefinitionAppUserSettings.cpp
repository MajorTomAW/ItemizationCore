// Author: Tom Werner (MajorT), 2025 November


#include "ItemDefinitionAppUserSettings.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemDefinitionAppUserSettings)

UItemDefinitionAppUserSettings::UItemDefinitionAppUserSettings()
{
}

UItemDefinitionAppUserSettings* UItemDefinitionAppUserSettings::Get()
{
	return GetMutableDefault<ThisClass>();
}
