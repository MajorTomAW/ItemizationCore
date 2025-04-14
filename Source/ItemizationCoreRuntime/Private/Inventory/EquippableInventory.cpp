// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/EquippableInventory.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EquippableInventory)

AEquippableInventory::AEquippableInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
{
}

void AEquippableInventory::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Fast Arrays don't use the push model, but there is no harm in marking them with it.
	// The flag will just be ignored.
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	Params.Condition = COND_None;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, EquippableInventory, Params);
	
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool AEquippableInventory::ReplicateSubobjects(
	UActorChannel* Channel,
	FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void AEquippableInventory::OnRep_EquippableInventory()
{
}
