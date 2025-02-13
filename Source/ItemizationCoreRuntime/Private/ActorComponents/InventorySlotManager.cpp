// Copyright © 2024 Playton. All Rights Reserved.


#include "ActorComponents/InventorySlotManager.h"

#include "ItemizationCoreLog.h"
#include "ActorComponents/InventoryManager.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InventorySlotManager)

using namespace Itemization;

UInventorySlotManager::UInventorySlotManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;

	bWantsInventoryData = true;

	SetIsReplicatedByDefault(true);
}

void UInventorySlotManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, SlotEntries);
}

UInventorySlotManager* UInventorySlotManager::GetSlotManager(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	UInventorySlotManager* Mgr = nullptr;
	Mgr = Actor->FindComponentByClass<UInventorySlotManager>();

	if (Mgr == nullptr)
	{
		if (const APawn* P = Cast<APawn>(Actor))
		{
			Mgr = P->GetController() ? P->GetController()->FindComponentByClass<UInventorySlotManager>() : nullptr;
			if (Mgr == nullptr)
			{
				Mgr = P->GetPlayerState() ? P->GetPlayerState()->FindComponentByClass<UInventorySlotManager>() : nullptr;
			}
		}
		else if (const AController* C = Cast<AController>(Actor))
		{
			Mgr = C->GetPawn() ? C->GetPawn()->FindComponentByClass<UInventorySlotManager>() : nullptr;
			if (Mgr == nullptr)
			{
				Mgr = C->GetPlayerState<APlayerState>() ? C->GetPlayerState<APlayerState>()->FindComponentByClass<UInventorySlotManager>() : nullptr;
			}
		}
	}

	return Mgr;
}

void UInventorySlotManager::InitializeComponent()
{
	Super::InitializeComponent();

	// Pre-allocate the slot entries
	SlotEntries.Slots.Reserve(DefaultSlotCount);
	for (int i = 0; i < DefaultSlotCount; ++i)
	{
		FInventorySlotEntry& SlotEntry = SlotEntries.Slots.AddDefaulted_GetRef();
		SlotEntry.SlotIndex = i;
	}
}

bool UInventorySlotManager::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void UInventorySlotManager::ReadyForReplication()
{
	Super::ReadyForReplication();
}

void UInventorySlotManager::GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const
{
	Super::GetReplicatedCustomConditionState(OutActiveState);
}

void UInventorySlotManager::GetAllSlotEntries(TArray<FInventorySlotEntry>& OutSlotEntries) const
{
	// Reset the output array
	OutSlotEntries.Empty(SlotEntries.Slots.Num());

	// Iterate over the slot entries and add them to the output array
	for (const FInventorySlotEntry& SlotEntry : SlotEntries.Slots)
	{
		OutSlotEntries.Add(SlotEntry);
	}
}