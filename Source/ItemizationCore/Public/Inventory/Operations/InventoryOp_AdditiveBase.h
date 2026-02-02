// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "GameplayTagContainer.h"
#include "InventoryTrackableOp.h"
#include "ItemizationCoreTags.h"

struct FInventoryItemEntry;
class UItemDefinitionBase;

/** Generic inventory operation for additive actions. Is "abstract" and not meant to be used directly. */
struct FInventoryOp_AdditiveBase : public FInventoryTrackableOp
{
public:
	struct FAdditiveParamsBase
	{
		virtual ~FAdditiveParamsBase() = default;
		
		/** The item entry to be moved/given. Can be null. */
		FInventoryItemEntry* ItemEntry = nullptr;
		
		/** The number of items to be added. */
		int32 NumItems = 0;

		/** The source object. */
		TWeakObjectPtr<UObject> SourceObject;

		/** The item definition. Can be null. */
		TWeakObjectPtr<const UItemDefinitionBase> ItemDefinition;

		/** The item instance. Can be null. */
		TWeakObjectPtr<UObject> ItemInstance;

		/** The slot group to add the item to. */
		FGameplayTag GroupTag = Itemization::Tags::TAG_InventoryGroup_Inventory;

		/** Optional context data for the move action. */
		FGameplayTagContainer* Context = nullptr;

		virtual FName GetParamsType() const { return "Abstract, override this"; };
	};
};
