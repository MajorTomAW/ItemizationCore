// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "EItemizationInventoryCreationType.generated.h"

/** Enum describing the policy for creating an inventory system. */
UENUM(BlueprintType)
enum class EItemizationInventoryCreationType : uint8
{
	/**
	 * The inventory system will be created at runtime, typically when the owning object is initialized.
	 * This is the default behavior for most inventory systems.
	 */
	Runtime = 0,

	/**
	 * The inventory system will be created at design time, typically when the owning object is placed in the level.
	 * This is useful for static inventories that don't need to be initialized dynamically.
	 */
	SetupData = 1,
	
	Invalid = 0xff 		UMETA(Hidden),
};
