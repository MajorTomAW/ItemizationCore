// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "EInventorySlotSelection.generated.h"

/**
 * Enum describing the selection type for inventory slots.
 */
UENUM(BlueprintType)
enum class EEInventorySlotSelection : uint8
{
	/** Selects a single slot. */
	Single,

	/** Selects a range of slots. */
	Range,

	/** Selects all slots. */
	All
};
