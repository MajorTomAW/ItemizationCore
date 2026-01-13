// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ItemDefinitionAppUserSettings.generated.h"

/**
 *
 */
UCLASS(MinimalAPI, Config=EditorPerProjectUserSettings, DisplayName="ItemDefinition User Settings")
class UItemDefinitionAppUserSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UItemDefinitionAppUserSettings();
	static UItemDefinitionAppUserSettings* Get();

public:
	UPROPERTY(Config, EditAnywhere, Category=UserSettings)
	bool bShowItemDataDescriptions = true;
};
