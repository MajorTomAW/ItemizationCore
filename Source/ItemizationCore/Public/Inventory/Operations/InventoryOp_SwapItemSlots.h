// Author: Tom Werner (dc: majort), 2026 February

#pragma once

#include "InventorySlotId.h"
#include "InventoryTrackableOp.h"

class ASlottableInventory;

struct FInventoryOp_SwapItemSlots : public FInventoryTrackableOp
{
	static constexpr TCHAR Name[] = TEXT("SwapItemSlots");

public:
	struct FParams
	{
		/** The first slot id to swap the contents from. */
		FInventorySlotId SourceSlotId;

		/** The source inventory. */
		TWeakObjectPtr<ASlottableInventory> SourceInventory;

		/** The source group tag. */
		FGameplayTag SourceGroupTag;

		/** The second slot id to swap the contents to. */
		FInventorySlotId TargetSlotId;

		/** The target inventory. */
		TWeakObjectPtr<ASlottableInventory> TargetInventory;

		/** The target group tag. */
		FGameplayTag TargetGroupTag;

		/** Makes a debug string. */
		FString GetDebugString() const
		{
			return FString::Printf(TEXT("(source slot: %s) <--> (target slot: %s)"),
				*SourceSlotId.ToString(),
				*TargetSlotId.ToString());
		}

		/** Checks whether these params are valid. */
		bool AreParamsValid() const
		{
			return SourceSlotId.IsValid() && TargetSlotId.IsValid() && SourceInventory.IsValid() && TargetInventory.IsValid();
		}

		/** Checks whether the source and target inventory are the same. */
		bool IsSameInventory() const { return SourceInventory == TargetInventory; }
	};

	struct FResult
	{
	};
};
