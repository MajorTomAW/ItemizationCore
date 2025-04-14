// Copyright © 2025 MajorT. All Rights Reserved.


#include "InventorySetupDataBase.h"

#include "Enums/EItemizationInventoryCreationType.h"

#include "Inventory/InventoryBase.h"
#include "Inventory/Inventory.h"
#include "Inventory/EquippableInventory.h"
#include "Inventory/SlottableInventory.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventorySetupDataBase)

UInventorySetupDataBase::UInventorySetupDataBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UInventorySetupDataBase_Default::UInventorySetupDataBase_Default(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SlottableInventory(TInstancedStruct<FSlottableInventoryProperties>::Make())
{
}

void UInventorySetupDataBase_Default::SpawnInventory(
	AActor* InOwner,
	EItemizationInventoryCreationType CreationType,
	AInventoryBase*& OutRootInventory)
{
	check(InOwner);
	check(InOwner->HasAuthority());

	UWorld* const World = InOwner->GetWorld();
	check(World);

	// Cache all spawned inventories for later initialization
	TArray<AInventoryBase*> SpawnedInventories;

	// Setup the spawn params
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = InOwner;
	SpawnInfo.Instigator = InOwner->GetInstigator();
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.ObjectFlags |= RF_Transient; // Runtime inventories should never be saved into a map

	// Spawn the inventories
	SpawnInventories(InventoryList, SpawnInfo, SpawnedInventories);

	// Spawn the equippable inventories
	SpawnInventories(EquippableInventoryList, SpawnInfo, SpawnedInventories);

	// Spawn the slottable inventories
	const TArray SlottableInventoryList = {SlottableInventory };
	SpawnInventories(SlottableInventoryList, SpawnInfo, SpawnedInventories);

	// Initialize all spawned inventories with the starting items
	TArray<const FInventoryStartingItem*> StartingItems;
	StartingItems.Reserve(SpawnedInventories.Num());
	for (const auto& Data : StartingItemList)
	{
		const FInventoryStartingItem* StartingItem = Data.GetPtr<FInventoryStartingItem>();
		if (ensure(StartingItem))
		{
			StartingItems.Add(StartingItem);
		}
	}

	AInventoryBase* RootInventory = nullptr;
	if (SpawnedInventories.IsValidIndex(0))
	{
		RootInventory = SpawnedInventories[0];
	}

	// Fill in the correct parent-child relationships
	if (ensure(RootInventory))
	{
		SpawnedInventories.RemoveSingle(RootInventory);
		RootInventory->Init(this, nullptr, SpawnedInventories);

		for (AInventoryBase* Inventory : SpawnedInventories)
		{
			Inventory->Init(this, RootInventory);
		}

		OutRootInventory = RootInventory;
	}

	// If the creation type is setup data, we need to initialize the inventories with the starting items
	/*if (CreationType == EItemizationInventoryCreationType::SetupData)
	{
		for (AInventoryBase* Inventory : SpawnedInventories)
		{
			Inventory->GrantStartingItems(StartingItems);
		}
	}*/
}