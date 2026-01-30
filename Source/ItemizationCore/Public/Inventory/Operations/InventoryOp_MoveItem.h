// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "InventoryTrackableOp.h"
#include "ItemizationCoreTags.h"
#include "Items/InventoryItemEntry.h"

struct FInventoryItemEntry;
class UItemDefinitionBase;
struct FGameplayTagContainer;
class AInventoryBase;

/**
 * Generic inventory item operation for moving items between inventories.
 */
struct FInventoryOp_MoveItem : public FInventoryTrackableOp
{
	static constexpr TCHAR Name[] = TEXT("MoveItem");

public:
	struct FParams
	{
		/** The source inventory to move the item from. */
		TWeakObjectPtr<AInventoryBase> SourceInventory;

		/** The target inventory to move the item to. */
		TWeakObjectPtr<AInventoryBase> TargetInventory;

		/** The number of items to move. */
		int32 NumMove = 0;

		/** The item entry to be moved. */
		FInventoryItemEntry* ItemEntry = nullptr;

		/** The slot group to move the item into. */
		FGameplayTag GroupTag = Itemization::Tags::TAG_InventoryGroup_Inventory;

		/** Makes a debug string. */
		FString GetDebugString() const
		{
			return FString::Printf(TEXT("(item: %s, count: %d)"),
				ItemEntry ? *ItemEntry->GetItemName().ToString() : TEXT("Invalid"),
				NumMove);
		}

		/** Checks whether this op has valid params. */
		bool AreParamsValid() const
		{
			return ItemEntry != nullptr && SourceInventory.IsValid() && TargetInventory.IsValid();
		}
	};

	struct FResult
	{
		/** The number of items that could not be moved due to stack size limits or other restrictions. */
		int32 NumCouldNotMove = 0;
	};
};