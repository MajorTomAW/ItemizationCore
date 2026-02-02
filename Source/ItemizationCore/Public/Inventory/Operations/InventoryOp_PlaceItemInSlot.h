// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "InventoryOp_AdditiveBase.h"

#include "GameplayTagContainer.h"
#include "InventorySlotId.h"
#include "ItemizationCoreTags.h"

class ASlottableInventory;
struct FInventoryItemEntry;

/** Operation for giving an placing an item into a slot. */
struct FInventoryOp_PlaceItemInSlot : public FInventoryOp_AdditiveBase
{
	static constexpr TCHAR Name[] = TEXT("PlaceItemInSlot");

public:
	struct FParams : public FAdditiveParamsBase
	{
		/** The specific slot this item wants to be placed in. DON'T READ, JUST WRITE! USE ResolveItemSlot() instead. */
		FInventorySlotId TargetSlotId;

		/** Makes a debug string. */
		FString GetDebugString() const
		{
			return FString::Printf(TEXT("(item: %s) -> (slot: %s)"),
				*GetItemNameString(),
				*TargetSlotId.ToString());
		}

		/** Checks whether these parameters count as valid. */
		bool AreParamsValid() const
		{
			return (ItemEntry != nullptr || ItemDefinition.IsValid() || ItemInstance.IsValid()) &&
				(TargetSlotId.IsValid()) &&
					NumItems > 0;
		}

		FString GetItemNameString() const
		{
			if (ItemEntry)
			{
				return ItemEntry->GetItemName().ToString();
			}

			if (ItemDefinition.IsValid())
			{
				return ItemDefinition->GetItemName().ToString();
			}
			
			if (ItemInstance.IsValid())
			{
				return GetNameSafe(ItemInstance.Get());
			}

			return TEXT("Invalid");
		}

		virtual FName GetParamsType() const override { return Name; }
	};

	struct FResult
	{
		/** Determines whether this op was successful (item could be placed in slot). */
		bool bSuccess = false;

		/** The number of items that could not be added/moved/removed due to stack size limits or other restrictions. */
		int32 Excess = 0;

		/** Handle to the item that was moved or acted upon. */
		FInventoryItemId ItemId = FInventoryItemId::InvalidId;
	};
};