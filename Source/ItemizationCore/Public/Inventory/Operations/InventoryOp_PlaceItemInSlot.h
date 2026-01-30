// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "GameplayTagContainer.h"
#include "InventorySlotId.h"
#include "InventoryTrackableOp.h"
#include "ItemizationCoreTags.h"

class ASlottableInventory;
struct FInventoryItemEntry;

struct FInventoryOp_PlaceItemInSlot : public FInventoryTrackableOp
{
	static constexpr TCHAR Name[] = TEXT("PlaceItemInSlot");

public:
	struct FParams
	{
		/** The item handle to be placed in the specified slot. */
		FInventoryItemEntry* ItemEntry = nullptr;

		/** The slot group to add the item to. */
		FGameplayTag GroupTag = Itemization::Tags::TAG_InventoryGroup_Inventory;

		/** The specific slot this item wants to be placed in. DON'T READ, JUST WRITE! USE ResolveItemSlot() instead. */
		FInventorySlotId TargetSlotId;

		/** The raw item slot. DON'T READ, JUST WRITE! USE ResolveItemSlot() instead.*/
		FInventoryItemSlot* TargetSlot = nullptr;

		FInventoryItemSlot* ResolveItemSlot(ASlottableInventory* TargetInventory) const;

		/** Makes a debug string. */
		FString GetDebugString() const
		{
			return FString::Printf(TEXT("(item: %s) -> (slot: %s)"),
				*ItemEntry->GetItemName().ToString(),
				*TargetSlot->GetDebugString());
		}

		/** Checks whether these parameters count as valid. */
		bool AreParamsValid() const
		{
			return ItemEntry != nullptr && (TargetSlotId.IsValid() || TargetSlot != nullptr) && GroupTag.IsValid();
		}
	};

	struct FResult
	{
		/** Determines whether this op was successful (item could be placed in slot). */
		bool bSuccess = false;
	};
};

inline FInventoryItemSlot* FInventoryOp_PlaceItemInSlot::FParams::ResolveItemSlot(ASlottableInventory* TargetInventory) const
{
	//@TODO: Uncomment
	unimplemented()
	return nullptr;
	/*if (!IsValid(TargetInventory))
	{
		return nullptr;
	}

	if (TargetSlot != nullptr)
	{
		return TargetSlot;
	}

	if (!TargetSlotHandle.IsValid())
	{
		return nullptr;
	}

	return TargetInventory->FindItemSlotByHandle(TargetSlotHandle);*/
}
