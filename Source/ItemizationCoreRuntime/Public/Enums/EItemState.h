// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "EItemState.generated.h"

/** 
 * Enum representing the state of an item in the inventory system.
 * This enum is used to track the current state of an item, such as whether it is in the inventory,
 * equipped, or not in the inventory at all.
 */
UENUM(BlueprintType)
enum class EItemState : uint8
{
	/** The item is currently not in any inventory. (e.g., dropped on the ground) */
	NotInInventory = 0,

	/** The item is in the inventory but not equipped. */
	InInventory = 1,

	/** The item is equipped and active. */
	EquippedAndActive = 2,
	
	MAX = 0xFF UMETA(Hidden),
};
