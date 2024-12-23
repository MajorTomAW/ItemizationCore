// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ItemComponentData.h"

#include "ItemComponentData_OwnedGameplayTags.generated.h"

/**
 * Item data for adding gameplay tags owned by the item.
 */
USTRUCT(DisplayName = "Owned Gameplay Tags Item Data", meta = (SingletonComponent = true))
struct FItemComponentData_OwnedGameplayTags : public FItemComponentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Tags;
};