// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/InventoryBase.h"

#include "ItemizationLogChannels.h"
#include "Enums/EItemizationInventoryType.h"

#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryBase)

//////////////////////////////////////////////////////////////////////////////////
/// AInventoryBase

AInventoryBase::AInventoryBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
	, InventoryType(EItemizationInventoryType::World)
	, ParentInventory(nullptr)
{
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bAlwaysRelevant = true;
	
	SetReplicatingMovement(false);
	SetNetUpdateFrequency(2.0f);
	SetMinNetUpdateFrequency(1.0f);
	SetNetCullDistanceSquared(112500000.0f);
	SetCanBeDamaged(false);
	SetHidden(true);
	SetActorEnableCollision(false);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bNetLoadOnClient = false;
	bFindCameraComponentWhenViewTarget = false;

#if WITH_EDITORONLY_DATA
	bHiddenEd = true;
#endif
}

void AInventoryBase::Init(
	AInventoryBase* InParent,
	const TArray<AInventoryBase*> InChildren)
{
	// Setup parent-children relationship
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ParentInventory, this);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ChildInventoryList, this);
	ParentInventory = InParent;
	ChildInventoryList = InChildren;

	// @TODO: Initialize the inventory with the setup data ??

	// Recursively initialize all child inventories
	for (AInventoryBase* Child : ChildInventoryList)
	{
		if (!ensure(Child))
		{
			continue;
		}
		
		Child->Init(this);
	}

	PostInitInventory();
}

void AInventoryBase::PostInitInventory()
{
}

void AInventoryBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	
	SharedParams.Condition = COND_InitialOnly;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ParentInventory, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ChildInventoryList, SharedParams);
	
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AInventoryBase::PostInitializeComponents()
{
	if (GetInstigatorController() && GetInstigatorController()->IsA<APlayerController>())
	{
		InventoryType = EItemizationInventoryType::Player;
	}
	
	Super::PostInitializeComponents();
	ITEMIZATION_N_LOG("Inventory Created!");
}

bool AInventoryBase::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void AInventoryBase::PreNetReceive()
{
	Super::PreNetReceive();
}