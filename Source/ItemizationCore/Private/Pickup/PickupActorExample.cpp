// Author: Tom Werner (dc: majort), 2026 January


#include "Pickup/PickupActorExample.h"

#include "Net/UnrealNetwork.h"
#include "Pickup/PickupCreationData.h"


APickupActorExample::APickupActorExample()
{
	bReplicates = true;
}

void APickupActorExample::SetupPickupWithCreationData(const FPickupCreationData& CreationData)
{
	PickupItem = CreationData.Get<FPickupCreationData_Impl>()->PickupItemEntry;
}

void APickupActorExample::DespawnPickup()
{
	Destroy();
}

const FInventoryItemEntry& APickupActorExample::GetPrimaryPickupItemEntry() const
{
	return PickupItem;
}

void APickupActorExample::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.Condition = COND_InitialOnly;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PickupItem, SharedParams)
}
