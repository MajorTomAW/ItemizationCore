// Copyright © 2024 Playton. All Rights Reserved.


#include "Settings/ItemizationCoreEditorSettings.h"

#include "ItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemizationCoreEditorSettings)

UItemizationCoreEditorSettings::UItemizationCoreEditorSettings()
{
	CommonItemClasses.Add(UItemDefinition::StaticClass());
}

UItemizationCoreEditorSettings* UItemizationCoreEditorSettings::Get()
{
	return GetMutableDefault<UItemizationCoreEditorSettings>();
}
