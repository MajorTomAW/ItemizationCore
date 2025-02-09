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

void UInventorySlotManager::SetInventoryManager(UInventoryManager* InInventoryManager)
{
	check(InInventoryManager);
	CachedInventoryData = InInventoryManager->GetInventoryData();

	ITEMIZATION_Net_LOG(Log, this, TEXT("Inventory Slot Manager (%s) linked to Inventory Manager (%s) [%s]"),
		*GetName(), *GetNameSafe(InInventoryManager), GetInventoryData().IsValid() ? *GetInventoryData()->ToString() : TEXT("null"));
}

void UInventorySlotManager::TryInitializeWithInventoryManager()
{
	if (AController* Controller = GetController())
	{
		if (UInventoryManager* MutableInventoryManager = UInventoryManager::GetInventoryManager(Controller))
		{
			SetInventoryManager(MutableInventoryManager);
			return;
		}

		// For non-autonomous proxies we need to create our own inventory data.
		if (!Controller->IsLocalController() || Controller->GetLocalRole() == ROLE_SimulatedProxy)
		{
			if (!CachedInventoryData.IsValid())
			{
				CachedInventoryData = MakeShareable(new FItemizationCoreInventoryData());
				CachedInventoryData->AvatarActor = GetPawn();
			}
		}
	}

	// If we are not able to find the inventory manager, we will try to find it again in the next frame
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		TryInitializeWithInventoryManager();
	}));
}

const TSharedPtr<FItemizationCoreInventoryData>& UInventorySlotManager::GetInventoryData() const
{
	return CachedInventoryData;
}

FItemizationCoreInventoryData* UInventorySlotManager::GetInventoryDataPtr() const
{
	return CachedInventoryData.Get();
}

UInventoryManager* UInventorySlotManager::GetInventoryManager() const
{
	if (CachedInventoryData.IsValid())
	{
		return CachedInventoryData->InventoryManager.Get();
	}

	return UInventoryManager::GetInventoryManager(GetController());
}

APawn* UInventorySlotManager::GetPawn() const
{
	if (APawn* Pawn = Cast<APawn>(GetOuter()))
	{
		return Pawn;
	}

	if (const AController* Controller = Cast<AController>(GetOuter()))
	{
		return Controller->GetPawn();
	}

	if (const APlayerState* PlayerState = Cast<APlayerState>(GetOuter()))
	{
		return PlayerState->GetPawn();
	}

	return nullptr;
}

AController* UInventorySlotManager::GetController() const
{
	if (AController* Controller = Cast<AController>(GetOuter()))
	{
		return Controller;
	}

	if (const APawn* Pawn = Cast<APawn>(GetOuter()))
	{
		return Pawn->GetController();
	}

	if (const APlayerState* PlayerState = Cast<APlayerState>(GetOuter()))
	{
		return PlayerState->GetOwner<AController>();
	}

	return nullptr;
}

bool UInventorySlotManager::IsOwnerActorAuthoritative() const
{
	if (!GetController())
	{
		return false;
	}

	return GetController()->HasAuthority();
}

void UInventorySlotManager::PreNetReceive()
{
	Super::PreNetReceive();

	TryInitializeWithInventoryManager();
}

void UInventorySlotManager::BeginPlay()
{
	Super::BeginPlay();

	TryInitializeWithInventoryManager();
}

void UInventorySlotManager::OnRegister()
{
	Super::OnRegister();

	TryInitializeWithInventoryManager();
}

void UInventorySlotManager::InitializeComponent()
{
	Super::InitializeComponent();

	TryInitializeWithInventoryManager();

	// Pre-allocate the slot entries
	SlotEntries.Slots.Reserve(DefaultSlotCount);
	for (int i = 0; i < DefaultSlotCount; ++i)
	{
		FInventorySlotEntry& SlotEntry = SlotEntries.Slots.AddDefaulted_GetRef();
		SlotEntry.SlotIndex = i;
	}
}

void UInventorySlotManager::UninitializeComponent()
{
	Super::UninitializeComponent();
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