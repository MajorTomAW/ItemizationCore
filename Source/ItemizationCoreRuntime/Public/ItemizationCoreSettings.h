// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ItemizationCoreSettings.generated.h"

UCLASS(Config = Engine, DefaultConfig)
class ITEMIZATIONCORERUNTIME_API UItemizationCoreSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UItemizationCoreSettings();
	static UItemizationCoreSettings* Get();

	//~ Begin UDeveloperSettings Interface
	virtual FName GetSectionName() const override;
	//~ End UDeveloperSettings Interface

public:
	/** List of item types that are also being registered as primary assets. */
	UPROPERTY(Config, EditAnywhere, Category = Items)
	TArray<FName> ItemTypes;
};
