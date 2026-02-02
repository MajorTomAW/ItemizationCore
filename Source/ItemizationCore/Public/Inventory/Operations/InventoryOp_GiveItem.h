// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryItemId.h"
#include "InventoryOp_AdditiveBase.h"

#include "Items/InventoryItemEntry.h"

struct FInventoryItemEntry;
class UItemDefinitionBase;
struct FGameplayTagContainer;
class AInventoryBase;

/**
 * Generic inventory item operation for giving items to inventories.
 */
struct FInventoryOp_GiveItem : public FInventoryOp_AdditiveBase
{
	static constexpr TCHAR Name[] = TEXT("GiveItem");

public:
	struct FParams : public FAdditiveParamsBase
	{
		/** Makes a debug string. */
		FString GetDebugString() const
		{
			return FString::Printf(TEXT("(item: %s, count: %d) -> (group: %s)"),
				ItemEntry ? *ItemEntry->GetItemName().ToString() : TEXT("invalid"),
				NumItems,
				*GroupTag.ToString());
		}

		/** Checks whether this op has valid params. */
		bool AreParamsValid() const
		{
			return (ItemEntry != nullptr || ItemDefinition.IsValid() || ItemInstance.IsValid()) &&
				NumItems > 0; 
		}

		virtual FName GetParamsType() const override { return Name; }
	};

	struct FResult
	{
		/** The number of items that could not be added/moved/removed due to stack size limits or other restrictions. */
		int32 Excess = 0;

		/** Handle to the item that was moved or acted upon. */
		FInventoryItemId ItemId = FInventoryItemId::InvalidId;
	};
};
