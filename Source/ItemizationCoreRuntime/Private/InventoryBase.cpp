// Copyright © 2025 MajorT. All Rights Reserved.


#include "InventoryBase.h"

#include "ItemizationLogChannels.h"
#include "Engine/ActorChannel.h"
#include "Enums/EItemizationInventoryType.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryBase)

AInventoryBase::AInventoryBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
	, InventoryType(EItemizationInventoryType::World)
	, ParentInventory(nullptr)
{
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicatingMovement(false);
	SetNetUpdateFrequency(1);

	bNetLoadOnClient = false;
}

void AInventoryBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_None;

	// If this is a player-controlled inventory, we should only replicate to the owner.
	if (InventoryType == EItemizationInventoryType::Player)
	{
		Params.Condition = COND_ReplayOrOwner;
	}

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryList, Params);
}

void AInventoryBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	const UWorld* World = GetWorld();

	ITEMIZATION_N_LOG("Inventory Created!");

	// Only continue if we're authoritative.
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}
}

bool AInventoryBase::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
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

void AInventoryBase::GrantStartingItems(TArray<const FInventoryStartingItem*> StartingItems)
{
}

void AInventoryBase::OnRep_InventoryList()
{
}
