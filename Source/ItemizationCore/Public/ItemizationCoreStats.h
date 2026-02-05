// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("ItemizationCore"), STATGROUP_ItemizationCore, STATCAT_Advanced);

#define DECLARE_ITEMIZATION_STAT(StatName, StatId) \
DECLARE_CYCLE_STAT_EXTERN(TEXT(StatName), STAT_Itemization_##StatId, STATGROUP_ItemizationCore, )

DECLARE_ITEMIZATION_STAT("Find Item Instance By Handle", FindItemInstanceByHandle);
DECLARE_ITEMIZATION_STAT("Find Item Entry By Handle", FindItemEntryByHandle);
DECLARE_ITEMIZATION_STAT("Find Item Instances In Group", FindItemInstancesInGroup);
DECLARE_ITEMIZATION_STAT("Find Item Slot By Handle", FindItemSlotByHandle);
DECLARE_ITEMIZATION_STAT("Give Item", GiveItem);
DECLARE_ITEMIZATION_STAT("Remove Item", RemoveItem);
DECLARE_ITEMIZATION_STAT("Place Item In Slot", PlaceItemInSlot);
DECLARE_ITEMIZATION_STAT("Swap Item Slots", SwapItemSlots);
DECLARE_ITEMIZATION_STAT("Drop Item", DropItem);

