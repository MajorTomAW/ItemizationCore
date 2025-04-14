// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/InventoryCollection.h"

#include "Enums/EItemizationInventoryType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryCollection)

AInventoryCollection::AInventoryCollection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
{
	InventoryType = EItemizationInventoryType::World;
}

AInventoryCollection::~AInventoryCollection()
{
	InventoryList.Empty();
	InventoryMap.Empty();
}

bool AInventoryCollection::CreateInventory(
	UObject* InOwner,
	FGuid& OutInventoryId,
	FInventoryItemContainer& OutInventory)
{
	if (!IsValid(InOwner))
	{
		return false;
	}

	FInventoryItemContainer NewInventory;
	//@TODO: Init the inventory with the setup data

	// Generate a new inventory ID
	OutInventoryId = FGuid::NewGuid();
	AddInventory_Internal(OutInventoryId, NewInventory);
	OutInventory = NewInventory;

	return true;
}

void AInventoryCollection::AddInventory_Internal(
	const FGuid& InventoryId,
	FInventoryItemContainer& Inventory)
{
	if (!ensure(InventoryId.IsValid()))
	{
		return;
	}

	InventoryList.Add(Inventory);
	InventoryMap.Add(InventoryId, &Inventory);
}
