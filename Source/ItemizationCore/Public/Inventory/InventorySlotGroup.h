// Author: Tom Werner (MajorT), 2025

#pragma once

#include "GameplayTagContainer.h"
#include "Items/InventoryItemSlot.h"

#include "InventorySlotGroup.generated.h"

USTRUCT()
struct FInventoryItemSlotGroup
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayTag GroupTag;

	UPROPERTY()
	TArray<FInventoryItemSlot> SlotList;
};