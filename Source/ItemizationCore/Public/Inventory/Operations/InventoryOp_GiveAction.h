// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryTrackableOp.h"
#include "InventoryItemHandle.h"
#include "InventorySlotHandle.h"
#include "ItemizationCoreTags.h"
#include "Items/InventoryItemEntry.h"

struct FInventoryItemEntry;
class UItemDefinitionBase;
struct FGameplayTagContainer;
class AInventoryBase;

/**
 * Generic inventory item operation for moving/giving/removing items between inventories.
 * @note:	This action uses a specific ItemEntry unlike its abstract version. @see FInventoryOp_ItemActionAbstract
 */
struct FInventoryOp_GiveAction : public FInventoryTrackableOp
{
	static constexpr TCHAR Name[] = TEXT("MoveItem");

public:
	struct FParams
	{
		/** The source inventory from which items are moved. */
		TWeakObjectPtr<AInventoryBase> SourceInventory;

		/** The target inventory to which items are moved. */
		TWeakObjectPtr<AInventoryBase> TargetInventory;

		/** The item entry to be moved/given. */
		FInventoryItemEntry* ItemEntry = nullptr;

		/** Optional context data for the move action. */
		FGameplayTagContainer* Context = nullptr;

		/** The item to be moved. */
		int32 NumGive = 0;

		/** The slot group to add the item to. */
		FGameplayTag GroupTag = Itemization::Tags::TAG_InventoryGroup_Inventory;

		/** Makes a debug string. */
		FString GetDebugString() const
		{
			return FString::Printf(TEXT("(item: %s, count: %d) -> (group: %s)"),
				*ItemEntry->GetItemName(),
				NumGive,
				*GroupTag.ToString());
		}
	};

	struct FResult
	{
		/** The number of items that could not be added/moved/removed due to stack size limits or other restrictions. */
		int32 Excess = 0;

		/** Handle to the item that was moved or acted upon. */
		FInventoryItemHandle ItemHandle = FInventoryItemHandle::InvalidHandle;
	};
};
