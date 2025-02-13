// Copyright © 2024 MajorT. All Rights Reserved.


#include "ActorComponents/InventoryExtensionComponent.h"

#include "ItemizationCoreLog.h"
#include "ItemizationCoreTypes.h"
#include "ActorComponents/InventoryManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryExtensionComponent)

using namespace Itemization;

UInventoryExtensionComponent::UInventoryExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	bWantsInventoryData = true;
	
	SetIsReplicatedByDefault(true);
}

UInventoryManager* UInventoryExtensionComponent::GetInventoryManager() const
{
	if (CachedInventoryData.IsValid())
	{
		return CachedInventoryData->InventoryManager.Get();
	}

	return UInventoryManager::GetInventoryManager(GetOwner());
}

void UInventoryExtensionComponent::SetInventoryManager(UInventoryManager* InInventoryManager)
{
	check(InInventoryManager);
	PreInitializeWithInventoryManager(InInventoryManager);
	
	CachedInventoryData = InInventoryManager->GetInventoryData();

	ITEMIZATION_Net_LOG(Log, this, TEXT("Inventory Extension Component (%s) is linked to Inventory Manager (%s)"),
		*GetName(), *InInventoryManager->GetName());
}

void UInventoryExtensionComponent::PreInitializeWithInventoryManager(UInventoryManager* InInventoryManager)
{
	// Override in derived classes.
}

void UInventoryExtensionComponent::TryInitializeWithInventoryManager()
{
	if (AActor* Owner = GetOwner())
	{
		// This only works for locally controlled pawns and authority.
		if (UInventoryManager* MutableInventoryMgr = UInventoryManager::GetInventoryManager(Owner))
		{
			SetInventoryManager(MutableInventoryMgr);
			return;
		}

		// For non-autonomous proxies we need to create our own inventory data.
		// We can't call Pawn->IsLocallyControlled() here as the pawn might not be set yet.
		const ENetMode NetMode = GetNetMode();
		const bool bIsLocal =
			(NetMode == NM_Standalone) // Not networked.
			|| (NetMode == NM_Client && Owner->GetLocalRole() == ROLE_AutonomousProxy) // Networked client in control.
			|| (Owner->GetRemoteRole() != ROLE_AutonomousProxy && Owner->GetLocalRole() == ROLE_Authority); // Local authority in control.

		if (!bIsLocal || Owner->GetLocalRole() == ROLE_SimulatedProxy)
		{
			if (!CachedInventoryData.IsValid())
			{
				CachedInventoryData = MakeShareable(new FItemizationCoreInventoryData());
				CachedInventoryData->OwnerActor = Owner;
				CachedInventoryData->AvatarActor = GetPawn();
			}
			return;
		}
	}

	// Try again next tick if we haven't found an inventory manager.
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		TryInitializeWithInventoryManager();
	}));
}

bool UInventoryExtensionComponent::IsOwnerActorAuthoritative() const
{
	if (!GetOwner())
	{
		return false;
	}

	return GetOwner()->HasAuthority();
}

APawn* UInventoryExtensionComponent::GetPawn() const
{
	// See if the owner already is a pawn.
	if (APawn* P = Cast<APawn>(GetOwner()))
	{
		return P;
	}

	// If not, try to get the pawn from the avatar.
	if (APawn* P = GetAvatarActor<APawn>())
	{
		return P;
	}

	// Last try, but this should never happen as the owner should mostly be a controller.
	if (APawn* P = GetOwnerActor<APawn>())
	{
		return P;
	}
	
	return nullptr;
}

AController* UInventoryExtensionComponent::GetController() const
{
	// See if the owner already is a controller.
	if (AController* C = Cast<AController>(GetOwner()))
	{
		return C;
	}

	// If not, try to get the controller from the owner.
	if (AController* C = GetOwnerActor<AController>())
	{
		return C;
	}

	// This shouldn't work as the controller isn't designed to be the avatar.
	if (AController* C = GetAvatarActor<AController>())
	{
		return C;
	}

	// Last try, get the controller from the pawn
	if (const APawn* P = GetPawn())
	{
		return P->GetController();
	}
	
	return nullptr;
}

AActor* UInventoryExtensionComponent::GetAvatarActor() const
{
	if (CachedInventoryData.IsValid())
	{
		if (CachedInventoryData->AvatarActor.IsValid())
		{
			return CachedInventoryData->AvatarActor.Get();
		}
	}

	if (const UInventoryManager* InventoryManager = GetInventoryManager())
	{
		return InventoryManager->GetAvatarActor();
	}

	return nullptr;
}

AActor* UInventoryExtensionComponent::GetOwnerActor() const
{
	if (CachedInventoryData.IsValid())
	{
		if (CachedInventoryData->OwnerActor.IsValid())
		{
			return CachedInventoryData->OwnerActor.Get();
		}
	}

	if (const UInventoryManager* InventoryManager = GetInventoryManager())
	{
		return InventoryManager->GetOwnerActor();
	}

	return nullptr;
}

void UInventoryExtensionComponent::PreNetReceive()
{
	Super::PreNetReceive();

	if (bWantsInventoryData)
	{
		TryInitializeWithInventoryManager();
	}
}

void UInventoryExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bWantsInventoryData)
	{
		TryInitializeWithInventoryManager();
	}
}

void UInventoryExtensionComponent::OnRegister()
{
	Super::OnRegister();

	if (bWantsInventoryData)
	{
		TryInitializeWithInventoryManager();
	}
}

void UInventoryExtensionComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (bWantsInventoryData)
	{
		TryInitializeWithInventoryManager();
	}
}

void UInventoryExtensionComponent::UninitializeComponent()
{
	Super::UninitializeComponent();

	if (CachedInventoryData.IsValid())
	{
		CachedInventoryData.Reset();
	}
}
