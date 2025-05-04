// Author: Tom Werner (MajorT), 2025

#pragma once

#include "EItemDataQueryResult.generated.h"

/** Enum representing the result of a query for an item data component. */
UENUM(BlueprintType)
enum class EItemDataQueryResult : uint8
{
	/** The query was successful and the item data was found. */
	Found =		0x0,

	/** The query was unsuccessful and the item data wasn't found. */
	NotFound =	0x1,
};