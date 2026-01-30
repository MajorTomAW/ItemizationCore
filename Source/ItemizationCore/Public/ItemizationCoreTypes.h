// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"

#include "ItemizationCoreTypes.generated.h"

/** Enum describing the type of inventory limit an inventory has. */
UENUM(BlueprintType)
enum class EInventoryLimitType : uint8
{
	/** The inventory is not limited. */
	None = 0,
	
	/** The inventory is limited by the total number of items inside it. */
	TotalItemCount,

	/** The inventory is limited by the  total number of all stack sizes together. */
	TotalStackSize,

	/** Will use custom IsInventoryLimitReached function. */
	Custom
};
