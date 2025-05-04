// Author: Tom Werner (MajorT), 2025

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
	NotInInventory =	0x00,

	/** The item is in the inventory but not equipped. */
	InInventory =		0x01,

	/** The item is equipped and active. */
	EquippedAndActive = 0x02,
	
	MAX =				0xFF	UMETA(Hidden),
};
