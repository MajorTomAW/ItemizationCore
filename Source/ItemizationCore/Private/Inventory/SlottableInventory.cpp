// Author: Tom Werner (dc: majort), 2026 January


#include "Inventory/SlottableInventory.h"

#include "ItemizationCoreLogChannels.h"
#include "ItemizationCoreStats.h"
#include "Net/UnrealNetwork.h"

ASlottableInventory::ASlottableInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SlotList(this)
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

void ASlottableInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.Condition = COND_ReplayOrOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, SlotList, SharedParams)
}

void ASlottableInventory::ProcessPlaceItemInSlotOperation(
	const TInventoryOpRef<FInventoryOp_PlaceItemInSlot>& PlaceItemInSlotOp)
{
	//@TODO
}
