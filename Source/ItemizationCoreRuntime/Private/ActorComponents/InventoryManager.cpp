// Copyright © 2025 MajorT. All Rights Reserved.


#include "ActorComponents/InventoryManager.h"

#include "Inventory/InventoryBase.h"
#include "InventorySetupDataBase.h"
#include "ItemizationLogChannels.h"
#include "Enums/EItemizationInventoryCreationType.h"
#include "Enums/EItemizationInventoryType.h"
#include "GameFramework/PlayerState.h"
#include "Inventory/Inventory.h"
#include "Inventory/Transactions/InventoryItemTransactionBase.h"
#include "Items/ItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryManager)

UInventoryManager::UInventoryManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RootInventory(nullptr)
	, CreationPolicy(EItemizationInventoryCreationType::SetupData)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;
}

AInventory* UInventoryManager::GetRootInventory() const
{
	if (RootInventory.IsValid())
	{
		return RootInventory.Get();
	}

	return nullptr;
}

#pragma region GiveItem()_Overloads
FInventoryItemHandle UInventoryManager::GiveItem(const FInventoryItemEntry& ItemEntry) const
{
	int32 DummyExcess = 0;
	return GiveItem(ItemEntry, DummyExcess);
}

FInventoryItemHandle UInventoryManager::K2_GiveItem(UItemDefinition* ItemDefinition, int32 StackCount, int32& OutExcess)
{
	// Build a new item entry
	FInventoryItemEntry ItemEntry = {ItemDefinition, StackCount, GetOwner()};

	// Validate the item entry
	if (!IsValid(ItemEntry.Definition))
	{
		ITEMIZATION_ERROR("Attempted to give an item with an invalid item definition.");
		return FInventoryItemHandle();
	}

	// Give the item and return the handle
	// This will perform any necessary validation and replication
	return GiveItem(ItemEntry, OutExcess);
}
#pragma endregion
FInventoryItemHandle UInventoryManager::GiveItem(const FInventoryItemEntry& ItemEntry, int32& OutExcess) const
{
	AActor* const Owner = GetOwner();
	if (!ensure(Owner))
	{
		return FInventoryItemHandle();
	}
	
	// Only the server can give items
	if (!Owner->HasAuthority())
	{
		ITEMIZATION_N_ERROR("Attempted to give an item on a non-authoritative actor. This is not allowed.");
		return FInventoryItemHandle();
	}

	// Make sure we only give valid item definitions
	if (!IsValid(ItemEntry.Definition))
	{
		ITEMIZATION_ERROR("Attempted to give an item with an invalid item definition. This is not allowed.");
		return FInventoryItemHandle();
	}

	// Start the GiveItem() transaction
	FInventoryTransaction_GiveItem Transaction;
	{
		Transaction.TargetInventory = GetRootInventory();
		Transaction.Instigator = Owner;
		Transaction.Delta = ItemEntry.StackCount;
		Transaction.ContextTags = nullptr;
	}
	return Transaction.TargetInventory->GiveItem(ItemEntry, Transaction, OutExcess);
}

void UInventoryManager::PostInitProperties()
{
	Super::PostInitProperties();
}

void UInventoryManager::InitializeComponent()
{
	Super::InitializeComponent();
}

void UInventoryManager::PostNetReceive()
{
	Super::PostNetReceive();
}

void UInventoryManager::OnRegister()
{
	Super::OnRegister();
}

void UInventoryManager::BeginPlay()
{
	Super::BeginPlay();
	
	// If we're a runtime inventory, we don't need to spawn anything as it has already been spawned by someone else.
	if (CreationPolicy != EItemizationInventoryCreationType::SetupData)
	{
		// Look for an inventory manager that has a valid inventory class.
		LookForInventoryManager();
		return;
	}
	
	AActor* Owner = GetOwner();
	if (!ensure(Owner))
	{
		return;
	}

	// We don't want to spawn the inventory classes on clients.
	const ENetMode NetMode = Owner->GetNetMode();
	if (IsNetSimulating() || NetMode == NM_Client)
	{
		return;
	}

	UWorld* const World = GetWorld();
	if (!World->IsGameWorld())
	{
		// No need to spawn inventories in the editor.
		return;
	}

	// Try to find a player controller
	APlayerController* PlayerController = Cast<APlayerController>(Owner);
	if (PlayerController == nullptr)
	{
		if (const APawn* Pawn = Cast<APawn>(Owner))
		{
			PlayerController = Pawn->GetController<APlayerController>();
		}
		else if (const APlayerState* PlayerState = Cast<APlayerState>(Owner))
		{
			PlayerController = PlayerState->GetPlayerController();
		}
	}

	// Spawn the inventory based on the setup data
	SpawnInventory(World, Owner, PlayerController);
}

void UInventoryManager::LookForInventoryManager()
{
}

void UInventoryManager::SpawnInventory(UWorld* World, AActor* Owner, APlayerController* PlayerController)
{
	check(World && World->IsGameWorld());
	check(Owner);
	check(CreationPolicy == EItemizationInventoryCreationType::SetupData);

	// If for some reason we already have a root inventory, we don't need to spawn anything.
	if (RootInventory.IsValid())
	{
		return;
	}

	// Load the setup data
	UInventorySetupDataBase* SetupPtr = InventorySetupData.LoadSynchronous();
	if (!ensure(SetupPtr))
	{
		return;
	}

	// Create the spawn info
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = PlayerController ? PlayerController->GetPawn() : Owner->GetInstigator();
	SpawnInfo.Owner = Owner;
	SpawnInfo.bNoFail = true;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.ObjectFlags |= RF_Transient; // Runtime inventories should never be saved into a map

	AInventory* RootInventoryPtr = nullptr;
	SetupPtr->SpawnInventory(SpawnInfo, PlayerController, RootInventoryPtr);

	if (ensure(RootInventoryPtr))
	{
		RootInventory = RootInventoryPtr;
	}
}
