// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ItemComponentData.h"

#include "ItemComponentData_Icon.generated.h"

/**
 * Item data for adding an icon for the item to display in the UI
 */
USTRUCT(meta = (CosmeticComponent = true, SingletonComponent = true), DisplayName = "UI Icon Item Data")
struct FItemComponentData_Icon : public FItemComponentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Icon")
	TSoftObjectPtr<UTexture2D> Icon;
};