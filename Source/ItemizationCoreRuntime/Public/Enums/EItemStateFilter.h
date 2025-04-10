// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "EItemStateFilter.generated.h"

/**
 * Enum representing the high-level filter for item states in the inventory system.
 */
UENUM(BlueprintType)
enum class EItemStateFilter : uint8
{
	/** The item is inside an inventory but not equipped. */
	Owned = 0					UMETA(DisplayName = "Owned"),

	/** The item is inside an equippable inventory but not primarily activated. */
	Equipped = 1				UMETA(DisplayName = "Equipped"),

	/** The item is inside an equippable inventory and is primarily activated. (e.g. in the main hand.) */
	EquippedAndActive = 2		UMETA(DisplayName = "Equipped and Active"),
};
