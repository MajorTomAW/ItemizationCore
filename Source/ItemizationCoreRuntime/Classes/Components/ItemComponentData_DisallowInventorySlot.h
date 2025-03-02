// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ItemComponentData.h"

#include "ItemComponentData_DisallowInventorySlot.generated.h"

/**
 * Item data for disallowing the item to be placed in a specific inventory slot.
 */
USTRUCT(DisplayName = "Disallow Slot Item Data", meta=(SingletonComponent=true))
struct FItemComponentData_DisallowInventorySlot : public FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData_DisallowInventorySlot();

public:
	/** If the slot has any of these tags, the item can't be placed in it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slot")
	FGameplayTagContainer DisallowedSlotTags;
};
