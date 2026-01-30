// Author: Tom Werner (dc: majort), 2026 January


#include "Inventory/SlottableInventory.h"

#include "ItemizationCoreLogChannels.h"
#include "ItemizationCoreStats.h"
#include "Net/UnrealNetwork.h"

ASlottableInventory::ASlottableInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventorySlotList(this)
{
}

TInventoryOpPtr<FInventoryOp_PlaceItemInSlot> ASlottableInventory::PlaceItemInSlot(
	FInventoryOp_PlaceItemInSlot::FParams&& Params)
{
	SCOPE_CYCLE_COUNTER(STAT_Itemization_PlaceItemInSlot);

	if (!Params.AreParamsValid())
	{
		ITEMIZATION_WARN("Called with invalid params [%s].", *Params.GetDebugString())
		return nullptr;
	}

	if (!HasAuthority())
	{
		ITEMIZATION_WARN("Called on an actor with no authority. Not allowed!")
		return nullptr;
	}

	if (Params.ResolveItemSlot(this) == nullptr)
	{
		ITEMIZATION_WARN("Unable to resolve item slot")
		return nullptr;
	}

	ITEMIZATION_LOG("Placing item (%s) in slot (%s) in inventory (%s)",
		*GetNameSafe(Params.ItemEntry->GetItemDefinition()),
		*Params.ResolveItemSlot(this)->GetSlotId().ToString(),
		*GetName())

	// Create the operation & process the operation
	TInventoryOpRef<FInventoryOp_PlaceItemInSlot> NewOp =
		MakeSharedOp<FInventoryOp_PlaceItemInSlot>(Params);
	ProcessPlaceItemInSlotOperation(NewOp);

	return NewOp;
}

FInventorySlotId ASlottableInventory::GetNextUnoccupiedSlotIdInGroup(FGameplayTag GroupTag) const
{
	return InventorySlotList.GetNextUnoccupiedSlotIdInGroup(GroupTag);
}

FInventoryItemSlot* ASlottableInventory::GetNextUnoccupiedSlotInGroup(const FGameplayTag& GroupTag) const
{
	return InventorySlotList.GetNextUnoccupiedSlotInGroup(GroupTag);
}

void ASlottableInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.Condition = COND_ReplayOrOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventorySlotList, SharedParams)
}

void ASlottableInventory::MarkItemSlotDirty(
	FInventoryItemSlot& ItemSlot,
	bool bWasAddOrChange,
	bool bForceMarkSlotDirty)
{
	if (HasAuthority())
	{
		if (bWasAddOrChange || bForceMarkSlotDirty)
		{
			InventorySlotList.MarkItemDirty(ItemSlot);
		}
		else
		{
			InventorySlotList.MarkArrayDirty();
		}
	}
	else
	{
		// Client-side, mark the entire array dirty so it will be replicated
		InventorySlotList.MarkArrayDirty();
	}
}

void ASlottableInventory::ProcessPlaceItemInSlotOperation(
	const TInventoryOpRef<FInventoryOp_PlaceItemInSlot>& PlaceItemInSlotOp)
{
	FInventoryOp_PlaceItemInSlot::FParams& Params = PlaceItemInSlotOp->Params;
	FInventoryOp_PlaceItemInSlot::FResult& Result = PlaceItemInSlotOp->Result;

	// Default to false, in case we early-out
	Result.bSuccess = false;

	// Resolve the item slot
	FInventoryItemSlot* ItemSlot = Params.ResolveItemSlot(this);
	check(ItemSlot)

	//@TODO: Swap slots? Idk what to do here right now, will decide in future
	ItemSlot->IsOccupied();

	// Assign the slot
	ItemSlot->AssignItemToSlot(*Params.ItemEntry);
}
