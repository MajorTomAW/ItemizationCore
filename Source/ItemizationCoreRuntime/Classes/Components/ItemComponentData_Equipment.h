// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ItemComponentData.h"

#include "ItemComponentData_Equipment.generated.h"

/**
 * Item data for allowing the item to be equipped by an actor.
 * You can set whether the item can be equipped or not, however, without this item data, the item won't be able to be equipped anyways.
 */
USTRUCT(meta = (CosmeticComponent = false, SingletonComponent = true), DisplayName = "Equipment Item Data")
struct FItemComponentData_Equipment : public FItemComponentData
{
	GENERATED_BODY()
	FItemComponentData_Equipment();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	uint32 bCanBeEquipped : 1;
};
