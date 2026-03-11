// Author: Tom Werner (dc: majort), 2026


#include "Pickup/ExamplePickupActor.h"

#include "Net/UnrealNetwork.h"
#include "Pickup/PickupCreationData.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ExamplePickupActor)

AExamplePickupActor::AExamplePickupActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	SetNetUpdateFrequency(5.f);
}

void AExamplePickupActor::SetupPickupWithCreationData(const FPickupCreationData& CreationData)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemEntry, this)
	ItemEntry = CreationData.Get<FPickupCreationData_Impl>()->PickupItemEntry;
}

void AExamplePickupActor::DespawnPickup()
{
	// Do stuff

	SetLifeSpan(0.5f);
}

const FInventoryItemEntry& AExamplePickupActor::GetPrimaryPickupItemEntry() const
{
	return ItemEntry;
}

void AExamplePickupActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	SharedParams.Condition = COND_InitialOnly;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemEntry, SharedParams)
}
