// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryTrackableOp.h"
#include "InventoryItemHandle.h"

struct FInventoryItemEntry;
class UItemDefinitionBase;
struct FGameplayTagContainer;
class AInventoryBase;

/**
 * Generic inventory item operation for moving/giving/removing items between inventories.
 * @note:	This action uses a specific ItemEntry unlike its abstract version. @see FInventoryOp_ItemActionAbstract
 */
struct FInventoryOp_ItemAction : public FInventoryTrackableOp
{
	static constexpr TCHAR Name[] = TEXT("MoveItem");

public:
	struct FParams
	{
		/** The source inventory from which items are moved. */
		TWeakObjectPtr<AInventoryBase> SourceInventory;

		/** The target inventory to which items are moved. */
		TWeakObjectPtr<AInventoryBase> TargetInventory;

		/** The item entry to be moved/given/removed. */
		FInventoryItemEntry* ItemEntry = nullptr;

		/** Optional context data for the move action. */
		FGameplayTagContainer* Context = nullptr;

		/** The item to be moved. */
		int32 Delta = 0;
	};

	struct FResult
	{
		/** The number of items that could not be added/moved/removed due to stack size limits or other restrictions. */
		int32 Excess = 0;

		/** Handle to the item that was moved or acted upon. */
		FInventoryItemHandle ItemHandle = FInventoryItemHandle::InvalidHandle;
	};
};
