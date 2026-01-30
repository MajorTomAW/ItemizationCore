// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryTrackableOp.h"
#include "InventoryItemId.h"
#include "ItemizationCoreTags.h"
#include "Items/InventoryItemEntry.h"

struct FInventoryItemEntry;
class UItemDefinitionBase;
struct FGameplayTagContainer;
class AInventoryBase;

/**
 * Generic inventory item operation for giving items to inventories.
 */
struct FInventoryOp_GiveItem : public FInventoryTrackableOp
{
	static constexpr TCHAR Name[] = TEXT("GiveItem");

public:
	struct FParams
	{
		/** The item entry to be moved/given. Can be null. */
		FInventoryItemEntry* ItemEntry = nullptr;

		/** Optional context data for the move action. */
		FGameplayTagContainer* Context = nullptr;

		/** The item to be added. */
		int32 NumGive = 0;

		/** The item definition. Can be null. */
		TWeakObjectPtr<const UItemDefinitionBase> ItemDefinition;

		/** The item instance. Can be null. */
		TWeakObjectPtr<UObject> ItemInstance;

		/** The source object. */
		TWeakObjectPtr<UObject> SourceObject;

		/** The slot group to add the item to. */
		FGameplayTag GroupTag = Itemization::Tags::TAG_InventoryGroup_Inventory;

		/** Makes a debug string. */
		FString GetDebugString() const
		{
			return FString::Printf(TEXT("(item: %s, count: %d) -> (group: %s)"),
				ItemEntry ? *ItemEntry->GetItemName().ToString() : TEXT("invalid"),
				NumGive,
				*GroupTag.ToString());
		}

		/** Checks whether this op has valid params. */
		bool AreParamsValid() const
		{
			return (ItemEntry != nullptr || ItemDefinition.IsValid() || ItemInstance.IsValid()) &&
				NumGive > 0; 
		}
	};

	struct FResult
	{
		/** The number of items that could not be added/moved/removed due to stack size limits or other restrictions. */
		int32 Excess = 0;

		/** Handle to the item that was moved or acted upon. */
		FInventoryItemId ItemId = FInventoryItemId::InvalidId;
	};
};
