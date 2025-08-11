// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryTrackableOp.h"
#include "InventoryItemHandle.h"

struct FInventoryItemEntry;
class UItemDefinitionBase;
struct FGameplayTagContainer;
class AInventoryBase;

/**
 * Abstract inventory item operation for removing items between inventories.
 * @note:	This operation has multiple ways of identifying the item to be removed.
 * If mulitple are valid, it will choose one in hierachical order:
 * - ItemEntry:			Most specific way to identify an item, used when the item is already known.
 * - ItemHandle:		Handle that may point to an item in the inventory. (could be invalid tho)
 * - ItemDefinition:	ItemDefinition that may match with multiple items in the inventory. Will choose the first one found.
 */
struct FInventoryOp_RemoveItem : public FInventoryTrackableOp
{
	static constexpr TCHAR Name[] = TEXT("RemoveItem");

public:
	struct FParams
	{
		/** The source inventory from which items are removed. */
		TWeakObjectPtr<AInventoryBase> SourceInventory;

		/** The target inventory to which items are removed. */
		TWeakObjectPtr<AInventoryBase> TargetInventory;

		/** Filter function to determine which items to remove. */
		TFunction<bool(const FInventoryItemEntry& Other)> FilterFunc;

		/** Optional context data for the remove action. */
		FGameplayTagContainer* Context = nullptr;

		/** The number of items to be removed (will always use the absolute value). */
		int32 Delta = 0;

		
		/** Helper function to check if any of the identifiers are valid. */
		inline bool HasValidFilterFunc() const
		{
			return FilterFunc.IsSet();
		}
	};

	struct FResult
	{
		/** The number of items that could not be removed due to not enough in the inventory or other restrictions. */
		int32 NumMissing = 0;
	};
};