// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("ItemizationCore"), STATGROUP_ItemizationCore, STATCAT_Advanced);

#define DECLARE_ITEMIZATION_STAT(StatName, StatId) \
	DECLARE_CYCLE_STAT_EXTERN(TEXT(#StatName), STAT_##StatId, STATGROUP_ItemizationCore, )

DECLARE_ITEMIZATION_STAT("Find Item Instance By Handle", FindItemInstanceByHandle);
DECLARE_ITEMIZATION_STAT("Find Item Entry By Handle", FindItemEntryByHandle);