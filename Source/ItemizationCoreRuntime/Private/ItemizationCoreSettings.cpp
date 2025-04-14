// Copyright © 2025 MajorT. All Rights Reserved.


#include "ItemizationCoreSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemizationCoreSettings)

UItemizationCoreSettings::UItemizationCoreSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryName = TEXT("Game");
	SectionName = TEXT("Itemization Core");
	
	ItemTypes = {"Item", "Test"};
}

UItemizationCoreSettings* UItemizationCoreSettings::Get()
{
	return GetMutableDefault<UItemizationCoreSettings>();
}