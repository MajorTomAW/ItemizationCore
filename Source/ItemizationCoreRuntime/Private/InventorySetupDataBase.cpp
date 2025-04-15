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
	, InventoryList({ TInstancedStruct<FInventoryProperties>::Make() })
	, SlottableInventoryList({ TInstancedStruct<FSlottableInventoryProperties>::Make() })
{
}

void UInventorySetupDataBase_Default::SpawnInventory(
	const FActorSpawnParameters& SpawnInfo,
	APlayerController* PlayerController,
	AInventory*& OutRootInventory)
{
	check(SpawnInfo.Owner);
	check(SpawnInfo.Owner->HasAuthority());

	UWorld* const World = SpawnInfo.Owner->GetWorld();
	check(World && World->IsGameWorld());

	// Cache all spawned inventories for later initialization
	TArray<AInventoryBase*> SpawnedInventories;

	// Actually spawn the inventories
	SpawnInventories(InventoryList, SpawnInfo, PlayerController, SpawnedInventories);
	SpawnInventories(EquippableInventoryList, SpawnInfo, PlayerController, SpawnedInventories);
	SpawnInventories(SlottableInventoryList, SpawnInfo, PlayerController, SpawnedInventories);

	// Should always be a valid pointer
	AInventory* RootInventory = (AInventory*)SpawnedInventories[0];
	
	TArray<AInventoryBase*> ChildInventories = SpawnedInventories;
	ChildInventories.RemoveSingle(RootInventory);

	// Init our root inventory which will then recursively init all child inventories
	RootInventory->Init(nullptr, ChildInventories);
	OutRootInventory = RootInventory;

	// Grant all initial items to the root inventory the next tick
	// All child inventory should now have bound to the root inventory's delegates.
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

	// Queue up a timer for the next world tick
	if (StartingItems.Num() > 0)
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([&]()
	{
		RootInventory->GrantStartingItems(StartingItems);
	}));	
	}
}
