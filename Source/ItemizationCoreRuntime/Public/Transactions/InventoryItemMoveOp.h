// Author: Tom Werner (MajorT), 2025

#pragma once


#include "InventoryTrackableOp.h"


struct FGameplayTagContainer;
class AInventoryBase;

struct FInventoryItemMoveOp : public FInventoryTrackableOp
{
	static constexpr TCHAR Name[] = TEXT("MoveItem");
	
public:
	struct Params
	{
		/** The source inventory from which items are moved. */
		TWeakObjectPtr<AInventoryBase> SourceInventory;

		/** The target inventory to which items are moved. */
		TWeakObjectPtr<AInventoryBase> TargetInventory;

		/** Optional context data for the move action. */
		FGameplayTagContainer* Context = nullptr;

		/** The item to be moved. */
		int32 Delta = 0;
	};

	struct Result
	{
		int32 Excess = 0;
	};
};

struct FInventoryItemGiveOp : public FInventoryTrackableOp
{
	static constexpr TCHAR Name[] = TEXT("GiveItem");

public:
	struct Params
	{
		/** The source inventory from which items are moved. */
		TWeakObjectPtr<AInventoryBase> SourceInventory;

		/** The target inventory to which items are moved. */
		TWeakObjectPtr<AInventoryBase> TargetInventory;

		/** Optional context data for the move action. */
		FGameplayTagContainer* Context = nullptr;
		
		/** The item to be given. */
		FName ItemId;

		/** The number of items to give. */
		int32 NumToGive = 0;
	};

	struct Result
	{
		int32 NumCouldNotGive = 0;
	};
};