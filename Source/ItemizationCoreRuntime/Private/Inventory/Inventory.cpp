// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/Inventory.h"

#include "Enums/EItemizationInventoryType.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Inventory)

AInventory::AInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
{
	// By default, this inventory should be considered private and not replicated to other clients.
	bOnlyRelevantToOwner = true;
}

void AInventory::GrantStartingItems(TArray<const FInventoryStartingItem*> StartingItems)
{
}

void AInventory::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Fast Arrays don't use the push model, but there is no harm in marking them with it.
	// The flag will just be ignored.
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_None;

	// If this is a player-controlled inventory, we should only replicate to the owner.
	if (InventoryType == EItemizationInventoryType::Player)
	{
		Params.Condition = COND_ReplayOrOwner;
	}

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryList, Params);

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool AInventory::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	check(Channel);
	check(Bunch);
	check(RepFlags);
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// Register the item instances
	/*for (UInventoryItemInstance* Instance : {})
	{
		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}*/

	return WroteSomething;
}

void AInventory::OnRep_InventoryList()
{
}
