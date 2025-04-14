// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/SlottableInventory.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SlottableInventory)

ASlottableInventory::ASlottableInventory(const FObjectInitializer& ObjectInitializer)
	: AInventoryBase(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
{
#if WITH_EDITORONLY_DATA
	bHiddenEd = true;
#endif

	SetNetUpdateFrequency(1.0f);
}

void ASlottableInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	SharedParams.Condition = COND_None;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventorySlots, SharedParams);
	
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ASlottableInventory::OnRep_InventorySlots()
{
}
