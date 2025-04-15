// Copyright © 2025 MajorT. All Rights Reserved.


#include "ActorComponents/InventoryManager.h"

#include "Inventory/InventoryBase.h"
#include "InventorySetupDataBase.h"
#include "ItemizationLogChannels.h"
#include "Enums/EItemizationInventoryCreationType.h"
#include "Inventory/Inventory.h"
#include "Inventory/Transactions/InventoryItemTransactionBase.h"
#include "Items/ItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryManager)

UInventoryManager::UInventoryManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RootInventory(nullptr)
	, CreationPolicy(EItemizationInventoryCreationType::Runtime)
	, RootInventoryClass(AInventory::StaticClass())
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
FInventoryItemHandle UInventoryManager::GiveItem(const FInventoryItemEntry& ItemEntry)
{
	int32 DummyExcess = 0;
	return GiveItem(ItemEntry, DummyExcess);
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

	
	// Attempt to create the inventory
	// Initialize the inventory manager with default values.
	UWorld* const World = GetWorld();
	if (!World->IsGameWorld())
	{
		// No need to spawn inventories in the editor.
		return;
	}

	AActor* Owner = GetOwner();
	if (GetNetMode() == NM_Client)
	{
		return;
	}

	// Create the inventory based on the setup data
	if (CreationPolicy == EItemizationInventoryCreationType::SetupData)
	{
		UInventorySetupDataBase* InventorySetupDataPtr = InventorySetupData.LoadSynchronous();

		// Create a new inventory if we don't have one.
		AInventoryBase* RootInventoryPtr = GetRootInventory();
		if (IsValid(InventorySetupDataPtr) && RootInventoryPtr == nullptr)
		{
			InventorySetupDataPtr->SpawnInventory(Owner, CreationPolicy, RootInventoryPtr);
		}

		ensureMsgf(RootInventoryPtr->GetOwner() == GetOwner(),
			TEXT("Created inventory %s with owner %s, but the inventory manager is owned by %s"), *RootInventoryPtr->GetName(), *RootInventoryPtr->GetOwner()->GetName(), *GetOwner()->GetName());

		RootInventory = (AInventory*)RootInventoryPtr;
	}
	// Otherwise use the default inventory class and create an empty one.
	else if (CreationPolicy == EItemizationInventoryCreationType::Runtime)
	{
		if (RootInventoryClass.IsNull())
		{
			ITEMIZATION_WARN("InventoryManager: No inventory class specified for runtime creation. Please set a class in the editor.");
			return;
		}

		UClass* Class = RootInventoryClass.LoadSynchronous();

		// Setup the spawn params
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = Owner;
		SpawnInfo.Instigator = Owner->GetInstigator();
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.ObjectFlags |= RF_Transient; // Runtime inventories should never be saved into a map

		RootInventory = World->SpawnActor<AInventory>(Class, SpawnInfo);
	}
}

void UInventoryManager::PostNetReceive()
{
	Super::PostNetReceive();
}

void UInventoryManager::OnRegister()
{
	Super::OnRegister();
}
