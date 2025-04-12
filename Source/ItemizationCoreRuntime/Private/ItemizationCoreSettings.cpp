// Copyright © 2025 MajorT. All Rights Reserved.


#include "ItemizationCoreSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemizationCoreSettings)

UItemizationCoreSettings::UItemizationCoreSettings()
{
	ItemTypes = {"Item", "Test"};
}

UItemizationCoreSettings* UItemizationCoreSettings::Get()
{
	return GetMutableDefault<UItemizationCoreSettings>();
}

FName UItemizationCoreSettings::GetSectionName() const
{
	return TEXT("Game");
}
