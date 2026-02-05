// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryTrackableOp.h"
#include "InventoryItemId.h"
#include "Items/ItemDefinitionBase.h"

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
		/** Filter function to determine which items to remove. Use only if you can't use definition/id/instance*/
		TFunction<bool(const FInventoryItemEntry& Other)> FilterFunc;

		/** The item id to remove. Can be null. */
		FInventoryItemId ItemId;

		/** The item definition to remove. Can be null. */
		TWeakObjectPtr<const UItemDefinitionBase> ItemDefinition;

		/** The item instance to remove. Can be null. */
		TWeakObjectPtr<UObject> ItemInstance;

		/** Optional context data for the remove action. */
		FGameplayTagContainer* Context = nullptr;

		/** The number of items to be removed (will always use the absolute value). */
		int32 NumRemove = 0;

		/** The slot group to add the item to. */
		FGameplayTag GroupTag = Itemization::Tags::TAG_InventoryGroup_Inventory;


		/** Helper function to check if any of the identifiers are valid. */
		inline bool HasValidFilterFunc() const
		{
			return FilterFunc.IsSet();
		}

		/** Makes a debug string. */
		inline FString GetDebugString() const
		{
			return FString::Printf(TEXT("(count: %d"),NumRemove);
		}

		/** Makes a debug string according to what filter we're using to find the item. */
		inline FString GetFilterDebugString() const
		{
			if (ItemId.IsValid())
			{
				return ItemId.ToString();
			}

			if (ItemDefinition.IsValid())
			{
				return GetNameSafe(ItemDefinition.Get());
			}

			if (ItemInstance.IsValid())
			{
				return GetNameSafe(ItemInstance.Get());
			}

			if (FilterFunc.IsSet())
			{
				return TEXT("Filter Func");
			}

			return TEXT("Invalid");
		}

		/** Checks whether this op has valid params. */
		bool AreParamsValid() const
		{
			return (ItemId.IsValid() || ItemDefinition.IsValid() || ItemInstance.IsValid() || FilterFunc.IsSet());
		}

		/** Checks whether we could technically filter more than one item. */
		bool MightHaveMultipleMatches() const
		{
			// Item id is unique
			if (ItemId.IsValid())
			{
				return false;
			}

			// Item instance is unique
			if (ItemInstance.IsValid())
			{
				return false;
			}

			// Filter func or item definition can have multiple matches
			return true;
		}
	};

	struct FResult
	{
		/** The number of items that could not be removed due to not enough in the inventory or other restrictions. */
		int32 NumMissing = 0;

		/** The number of items that got removed. */
		int32 NumRemoved = 0;

		/** True if any items were removed. */
		bool bRemovedAny = false;
	};
};
