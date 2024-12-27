// Copyright Epic Games, Inc. All Rights Reserved.


#include "ActorComponents/EquipmentManager.h"

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentManager)

UEquipmentManager::UEquipmentManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;

	SetIsReplicatedByDefault(true);
}

UEquipmentManager* UEquipmentManager::GetEquipmentManager(AActor* Actor)
{
	void* Mgr = nullptr;
	if (Actor)
	{
		Mgr = Actor->GetComponentByClass<UEquipmentManager>();
	}

	if (Mgr == nullptr)
	{
		if (const AController* C = Cast<AController>(Actor))
		{
			Mgr = C->GetPawn()->GetComponentByClass<UEquipmentManager>();
		}
		else if (const APlayerState* PS = Cast<APlayerState>(Actor))
		{
			Mgr = PS->GetPawn()->GetComponentByClass<UEquipmentManager>();
		}
	}

	return (UEquipmentManager*)Mgr;
}

void UEquipmentManager::PreNetReceive()
{
	Super::PreNetReceive();
}

void UEquipmentManager::BeginPlay()
{
	Super::BeginPlay();
}

void UEquipmentManager::OnRegister()
{
	Super::OnRegister();
}

void UEquipmentManager::InitializeComponent()
{
	Super::InitializeComponent();
}

void UEquipmentManager::UninitializeComponent()
{
	Super::UninitializeComponent();
}

bool UEquipmentManager::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void UEquipmentManager::ReadyForReplication()
{
	Super::ReadyForReplication();
}

void UEquipmentManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_None;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, EquipmentList, Params);
	
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UEquipmentManager::GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const
{
	Super::GetReplicatedCustomConditionState(OutActiveState);
}
