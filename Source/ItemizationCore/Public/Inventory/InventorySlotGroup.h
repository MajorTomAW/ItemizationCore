// Author: Tom Werner (MajorT), 2025

#pragma once

#include "GameplayTagContainer.h"

#include "InventorySlotGroup.generated.h"

struct FInventoryItemSlot;

USTRUCT()
struct FInventoryItemSlotGroup
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayTag GroupTag;

	/** List of slot pointers. */
	UPROPERTY()
	TArray<FInventoryItemSlot> SlotList;
};