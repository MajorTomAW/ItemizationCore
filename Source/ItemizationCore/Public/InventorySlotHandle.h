// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventorySlotHandle.generated.h"

USTRUCT(BlueprintType)
struct alignas(8) FInventorySlotHandle
{
	GENERATED_BODY()

	FInventorySlotHandle() = default;

public:
	UPROPERTY()
	int32 RowIndex = 0;

	UPROPERTY()
	int32 ColumnIndex = 0;
};

static_assert(sizeof(FInventorySlotHandle) == sizeof(uint64), "Expected FInventorySlotHandle to be 8 bytes.");
static_assert(alignof(FInventorySlotHandle) == sizeof(uint64), "Expected FInventorySlotHandle to be aligned to 8 bytes.");