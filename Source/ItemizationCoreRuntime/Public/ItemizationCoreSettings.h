// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ItemizationCoreSettings.generated.h"

/** Configure the settings for the itemization core system. */
UCLASS(Config = Engine, DefaultConfig, DisplayName="Itemization Core", MinimalAPI)
class UItemizationCoreSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UItemizationCoreSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	ITEMIZATIONCORERUNTIME_API static UItemizationCoreSettings* Get();

public:
	/** List of item types that are also being registered as primary assets. */
	UPROPERTY(Config, EditAnywhere, Category = Items)
	TArray<FName> ItemTypes;
};
