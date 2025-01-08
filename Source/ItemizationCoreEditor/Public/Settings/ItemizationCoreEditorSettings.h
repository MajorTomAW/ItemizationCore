// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ItemizationCoreEditorSettings.generated.h"

/**
 * Developer settings for the itemization system.
 */
UCLASS(Config = Editor, DefaultConfig)
class ITEMIZATIONCOREEDITOR_API UItemizationCoreEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UItemizationCoreEditorSettings();
	static UItemizationCoreEditorSettings* Get();

public:
	/** List of classes that are common for items. */
	UPROPERTY(Config, EditAnywhere, Category = "Item Classes", meta = (AllowedClasses = "/Script/ItemizationCoreRuntime.ItemDefinition"))
	TArray<TObjectPtr<UClass>> CommonItemClasses;
};
