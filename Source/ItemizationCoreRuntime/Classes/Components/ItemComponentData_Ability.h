// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ItemComponentData.h"

#include "ItemComponentData_Ability.generated.h"



USTRUCT(DisplayName = "Ability Item Data", meta = (SingletonComponent = true))
struct FItemComponentData_Ability : public FItemComponentData
{
	GENERATED_BODY()

public:
};