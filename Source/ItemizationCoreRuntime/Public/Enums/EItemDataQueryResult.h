// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "EItemDataQueryResult.generated.h"

/** Enum representing the result of a query for an item data component. */
UENUM(BlueprintType)
enum class EItemDataQueryResult : uint8
{
	/** The query was successful and the item data was found. */
	Found = 0,

	/** The query was unsuccessful and the item data wasn't found. */
	NotFound = 1,
};
