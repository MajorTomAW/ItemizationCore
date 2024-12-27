// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationCoreTypes.h"

#include "InventoryItemInstance.h"
#include "ItemizationCoreLog.h"
#include "ActorComponents/InventoryManager.h"
#include "GameFramework/PlayerState.h"

FString FItemActionContextData::ToString() const
{
	return FString::Printf(TEXT("Context Data:\n    Instigator: %s\n    InventoryManager: %s\n    ContextTags: %s\n    Delta: %d\n    SlotSize: %d\n    MaxStackSize: %d"),
			*GetNameSafe(Instigator.Get()), *GetNameSafe(InventoryManager.Get()), ContextTags ? *ContextTags->ToString() : TEXT("Null"), Delta, SlotSize, MaxStackSize);
}

FInventoryChangeMessage::FInventoryChangeMessage(UInventoryItemInstance* ItemThatChanged, const int32 OldCount, const int32 NewCount)
{
	Item = ItemThatChanged;
	NewStackCount = NewCount;
	OldStackCount = OldCount;
	Delta = NewCount - OldCount;
	InventoryManager = ItemThatChanged ? ItemThatChanged->GetOwningInventoryManager() : nullptr;
	Owner = ItemThatChanged ? ItemThatChanged->GetTypedOuter<AActor>() : nullptr;

	if (AController* C = Cast<AController>(Owner))
	{
		Controller = C;
	}
	else if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		Controller = Pawn->GetController();
	}
	else if (const APlayerState* PS = Cast<APlayerState>(Owner))
	{
		Controller = PS->GetOwningController();
	}
}

FItemizationCoreInventoryData::FItemizationCoreInventoryData()
	: OwnerActor(nullptr)
	, AvatarActor(nullptr)
	, Controller(nullptr)
	, InventoryManager(nullptr)
{
}

bool FItemizationCoreInventoryData::IsLocallyControlled() const
{
	if (const AController* C = Controller.Get())
	{
		return C->IsLocalController();
	}

	if (const APawn* P = Cast<APawn>(OwnerActor))
	{
		if (P->IsLocallyControlled())
		{
			return true;
		}

		if (P->GetController())
		{
			return false;
		}
	}

	return HasNetAuthority();
}

bool FItemizationCoreInventoryData::IsLocallyControlledPlayer() const
{
	if (const APlayerController* PC = Cast<APlayerController>(Controller.Get()))
	{
		return PC->IsLocalController();
	}

	return false;
}

bool FItemizationCoreInventoryData::HasNetAuthority() const
{
	// Make sure this works on pending kill actors too
	if (const AActor* const OwnerActorPtr = OwnerActor.Get(true))
	{
		return (OwnerActorPtr->GetLocalRole() == ROLE_Authority);
	}

	// This should never happen, maybe on shutdown?
	ITEMIZATION_LOG(Warning, TEXT("HasNetAuthority called when OwnerActor was invalid. Returning false. InventoryManager: %s"), *GetNameSafe(InventoryManager.Get()));
	return false;
}

void FItemizationCoreInventoryData::InitFromActor(
	AActor* InOwnerActor, AActor* InAvatarActor, UInventoryManager* InInventoryManager)
{
	check(InOwnerActor);
	check(InInventoryManager);

	OwnerActor = InOwnerActor;
	AvatarActor = InAvatarActor;
	InventoryManager = InInventoryManager;

	AController* OldC = Controller.Get();

	// Look for a controller or a pawn in the owner chain.
	AActor* TestActor = InOwnerActor;
	while (TestActor)
	{
		if (AController* C = Cast<AController>(TestActor))
		{
			Controller = C;
			break;
		}

		if (const APawn* P = Cast<APawn>(TestActor))
		{
			Controller = P->GetController();
			break;
		}

		TestActor = TestActor->GetOwner();
	}

	// Notify the inventory system if a new controller was found for the first time.
	if (OldC == nullptr && Controller.IsValid())
	{
		InInventoryManager->OnControllerSet();
	}
}

void FItemizationCoreInventoryData::SetAvatarActor_Direct(AActor* NewAvatarActor)
{
}

void FItemizationCoreInventoryData::ClearInventoryData()
{
}
