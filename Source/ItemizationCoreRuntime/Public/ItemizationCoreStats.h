// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("ItemizationCore"), STATGROUP_ItemizationCore, STATCAT_Advanced);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Find Item Entry From Handle"), STAT_FindItemEntryFromHandle, STATGROUP_ItemizationCore, );
DECLARE_CYCLE_STAT_EXTERN(TEXT("Find Equipment Entry From Handle"), STAT_FindEquipmentEntryFromHandle, STATGROUP_ItemizationCore, );
DECLARE_CYCLE_STAT_EXTERN(TEXT("Evaluate Current Item Context"), STAT_EvaluateCurrentItemContext, STATGROUP_ItemizationCore, );
DECLARE_CYCLE_STAT_EXTERN(TEXT("Can Create New Stack"), STAT_CanCreateNewStack, STATGROUP_ItemizationCore, );
DECLARE_CYCLE_STAT_EXTERN(TEXT("Can Combine Item Entries"), STAT_CanCombineItemEntries, STATGROUP_ItemizationCore, );