// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "EItemComponentInstancingPolicy.generated.h"

/**
 * Enum describing the instancing behavior of item components.
 */
UENUM(BlueprintType)
enum class EItemComponentInstancingPolicy : uint8
{
	/** The item component is never instanced. Any modifiers on this component will be shared among all referencing item definitions. */
	NonInstanced,

	/** */
	InstancedPerItem,
};
