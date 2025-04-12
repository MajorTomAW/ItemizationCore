// Copyright © 2025 MajorT. All Rights Reserved.


#include "InventorySetupDataBase.h"

#include "InventoryBase.h"
#include "Enums/EItemizationInventoryCreationType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventorySetupDataBase)

UInventorySetupDataBase::UInventorySetupDataBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FInventoryPropertiesBase::FInventoryPropertiesBase()
	: TotalSlotsOverride(INDEX_NONE)
{
}

UInventorySetupDataBase_Default::UInventorySetupDataBase_Default(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
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
	for (const auto& Data : InventoryList)
	{
		const FInventoryProperties* Props = Data.GetPtr<FInventoryProperties>();
		if (!ensure(Props))
		{
			continue;
		}
		
		TSoftClassPtr<AInventoryBase> InventoryClass = Props->InventoryClass;
		if (!ensure(!InventoryClass.IsNull()))
		{
			continue;
		}

		UClass* const Class = InventoryClass.LoadSynchronous();
		AInventoryBase* Inventory = World->SpawnActor<AInventoryBase>(Class, SpawnInfo);
		SpawnedInventories.Add(Inventory);
	}

	// Spawn the equippable inventories

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

	// If the creation type is setup data, we need to initialize the inventories with the starting items
	if (CreationType == EItemizationInventoryCreationType::SetupData)
	{
		for (AInventoryBase* Inventory : SpawnedInventories)
		{
			Inventory->GrantStartingItems(StartingItems);
		}
	}

	if (SpawnedInventories.IsValidIndex(0))
	{
		OutRootInventory = SpawnedInventories[0];
	}
}
