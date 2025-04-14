// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/InventoryBase.h"

#include "ItemizationLogChannels.h"
#include "Engine/ActorChannel.h"
#include "Enums/EItemizationInventoryType.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

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
	SetNetUpdateFrequency(1.0f);
	SetCanBeDamaged(false);
	SetHidden(true);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bNetLoadOnClient = false;

#if WITH_EDITORONLY_DATA
	bHiddenEd = true;
#endif
}

void AInventoryBase::Init(
	const UInventorySetupDataBase* InSetupData,
	AInventoryBase* InParent,
	const TArray<AInventoryBase*> InChildren)
{
	// Setup parent-children relationship
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ParentInventory, this);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ChildInventoryList, this);
	ParentInventory = InParent;
	ChildInventoryList = InChildren;

	// @TODO: Initialize the inventory with the setup data ??
}

void AInventoryBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	SharedParams.Condition = COND_None;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ParentInventory, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ChildInventoryList, SharedParams);
	
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}