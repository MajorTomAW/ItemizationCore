// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/InventoryDescriptor.h"

#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

#include "Inventory/Inventory.h"

////////////////////////////////////////////////////////////////////////////////
/// FInventoryDescriptorData

FInventoryDescriptorData::FInventoryDescriptorData(AActor* InOwner, AActor* InAvatar, AInventory* InInventory)
	: InventoryOwner(InOwner)
	, InventoryAvatar(InAvatar)
	, OwningInventory(InInventory)
{
	if (const APawn* P = Cast<APawn>(InOwner))
	{
		PlayerController = P->GetController<APlayerController>();
	}
	else if (APlayerController* AsPC = Cast<APlayerController>(InOwner))
	{
		PlayerController = AsPC;
	}
}

void FInventoryDescriptorData::InitFromActor(AActor* InOwner, AActor* InAvatar, AInventory* InInventory)
{
	check(InOwner);

	// Fallback to the owner if avatar is not set
	if (InAvatar == nullptr)
	{
		InAvatar = InOwner;
	}

	InventoryOwner = InOwner;
	InventoryAvatar = InAvatar;
	OwningInventory = InInventory;
	
	APlayerController* OldPlayerController = PlayerController.Get();

	// Look for a player controller or pawn in the owner chain
	AActor* TestActor = InOwner;
	while (TestActor)
	{
		if (APlayerController* PotPlayerController = Cast<APlayerController>(TestActor))
		{
			PlayerController = PotPlayerController;
			break;
		}

		if (const APawn* Pawn = Cast<const APawn>(TestActor))
		{
			PlayerController = Cast<APlayerController>(Pawn->GetController());
			break;
		}

		TestActor = TestActor->GetOwner();
	}

	// Notify the inventory about the controller change to re-assign inputs
	if (OldPlayerController == nullptr && PlayerController.IsValid())
	{
		
	}
}

void FInventoryDescriptorData::SetInventoryAvatar(AActor* InNewAvatar)
{
	InitFromActor(InventoryOwner.Get(), InNewAvatar, OwningInventory.Get());
}

bool FInventoryDescriptorData::HasAuthority() const
{
	return GetLocalRole() == ROLE_Authority;
}

ENetRole FInventoryDescriptorData::GetLocalRole() const
{
	if (AActor const* Owner = InventoryOwner.Get())
	{
		return Owner->GetLocalRole();
	}

	return ROLE_None;
}

void FInventoryDescriptorData::Reset()
{
	InventoryOwner = nullptr;
	InventoryAvatar = nullptr;
	PlayerController = nullptr;
	OwningInventory = nullptr;
}