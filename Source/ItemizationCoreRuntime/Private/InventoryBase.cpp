// Copyright © 2025 MajorT. All Rights Reserved.


#include "InventoryBase.h"

#include "Enums/EItemizationInventoryType.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryBase)

AInventoryBase::AInventoryBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryType(EItemizationInventoryType::World)
	, ParentInventory(nullptr)
{
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

void AInventoryBase::OnRep_InventoryList()
{
}
