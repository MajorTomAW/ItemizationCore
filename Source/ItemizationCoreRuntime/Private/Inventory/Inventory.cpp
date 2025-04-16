// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/Inventory.h"

#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

#include "Enums/EItemizationInventoryType.h"
#include "Inventory/Transactions/InventoryItemTransactionBase.h"
#include "Items/ItemDefinition.h"
#include "InventoryItemHandle.h"
#include "InventorySlotHandle.h"
#include "ItemizationLogChannels.h"
#include "Inventory/Messaging/InventoryChangeMessage.h"


#include "Items/ComponentData/ItemComponentData_MaxStackSize.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Inventory)

AInventory::AInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
	, InventoryList(this)
{
	// By default, this inventory should be considered private and not replicated to other clients.
	bOnlyRelevantToOwner = true;
}

void AInventory::GrantStartingItems(TArray<const FInventoryStartingItem*> StartingItems)
{
}

FInventoryItemHandle AInventory::GiveItem(
	const FInventoryItemEntry& ItemEntry,
	const FInventoryTransaction_GiveItem& Transaction,
	int32& OutExcess)
{
	check(ItemEntry.Definition);
	const FItemComponentData_MaxStackSize* MaxStackData = ItemEntry.Definition->GetItemComponent<FItemComponentData_MaxStackSize>();
	
	// The instigator should always be valid
	//@TODO: Should it really?? What happens if a player or object got destroyed?
	if (!ensure(Transaction.Instigator.IsValid()))
	{
		return FInventoryItemHandle();
	}

	// If locked, add to the pending list
	//@TODO: Implement the pending list

	// Evaluate the item entry & transaction
	FInventoryTransaction_GiveItem& MutableTransaction = const_cast<FInventoryTransaction_GiveItem&>(Transaction);
	EvaluateItemEntry(ItemEntry, MutableTransaction);

	ITEMIZATION_DISPLAY("Giving item [%s] %s. Stack Count: %d (Max: %d), Source: %s",
		*ItemEntry.ItemHandle.ToString(),
		*GetNameSafe(ItemEntry.Definition),
		Transaction.Delta,
		MaxStackData ? MaxStackData->GetMaxStackSize() : Transaction.Delta,
		*GetNameSafe(ItemEntry.SourceObject.Get()));

	return NativeGiveItem(ItemEntry, Transaction, OutExcess);
}

FInventoryItemHandle AInventory::NativeGiveItem(
	const FInventoryItemEntry& ItemEntry,
	const FInventoryTransaction_GiveItem& Transaction,
	int32& Excess)
{
	FInventoryItemHandle LastHandle;
	FInventoryTransaction_GiveItem& MutableTransaction = const_cast<FInventoryTransaction_GiveItem&>(Transaction);

	// Evaluate the max stack size. @TODO: Do we really have to manually search for this component data?
	const FItemComponentData_MaxStackSize* MaxStackData =
		ItemEntry.Definition->GetItemComponent<FItemComponentData_MaxStackSize>();
	const int32 MaxStackSize = MaxStackData ? MaxStackData->GetMaxStackSize() : 1;

	// Start by setting the excess to the delta
	Excess = Transaction.Delta;

	// If we have excess items, try to add them to the inventory
	while (Excess > 0)
	{
		const int32 Delta = FMath::Min(Excess, MaxStackSize);
		Excess -= Delta;
		
		FInventoryItemEntry EntryCopy = ItemEntry;
		EntryCopy.StackCount = Delta;

		FInventoryItemEntry& NewItem = InventoryList.Items.Add_GetRef(EntryCopy);
		NewItem.ItemHandle.GenerateNewUID();

		LastHandle = NewItem.ItemHandle;

		// Create a new instance server-side
		if (ShouldCreateNewInstanceOfItem(NewItem))
		{
			CreateNewInstanceOfItem(NewItem);
		}

		
		OnGiveItem(NewItem);
		MarkItemEntryDirty(NewItem, true);	// Mark dirty for replication
	}

	Excess = FMath::Max(0, Excess);
	return LastHandle;
}

bool AInventory::ShouldCreateNewInstanceOfItem(const FInventoryItemEntry& ItemEntry) const
{
	// Always return true, can be overridden in subclasses
	//@TODO: Maybe some items dont require having an instance??
	return true;
}

UInventoryItemInstance* AInventory::CreateNewInstanceOfItem(FInventoryItemEntry& ItemEntry)
{
	checkf(ItemEntry.GetInstance() == nullptr, TEXT("Item instance already exists for item '%s'!"),
		*ItemEntry.GetDebugString());

	const UItemDefinition* Definition = ItemEntry.Definition;
	check(Definition);

	//@TODO: Add support for custom instances
	const UClass* InstanceClass = UInventoryItemInstance::StaticClass();

	// Create the new instance
	UInventoryItemInstance* NewInstance = NewObject<UInventoryItemInstance>(this, InstanceClass);
	check(NewInstance);

	// Add it to our instances-list so that it doesn't get garbage collected
	if (NewInstance->GetIsReplicated())
	{
		AddReplicatedItemInstance(NewInstance);
		ItemEntry.ReplInstance = NewInstance;
	}
	else
	{
		ItemEntry.LocalInstance = NewInstance;
	}

	return NewInstance;
}

void AInventory::OnRemoveItem(FInventoryItemEntry& ItemEntry)
{
	UInventoryItemInstance* Instance = ItemEntry.GetInstance();
	if (Instance != nullptr)
	{
		// Unregister the item instance
	}

	// Broadcast the change event
	FInventoryChangeMessage Payload;
	{
		Payload.Controller = GetInstigatorController();
		Payload.Owner = GetOwner();
		Payload.Inventory = this;
		Payload.ItemEntry = &ItemEntry;
		Payload.NewStackCount = 0;
		Payload.Delta = -ItemEntry.StackCount;
	}
	
	OnItemRemovedDelegate.Broadcast(Payload);
}

void AInventory::OnGiveItem(FInventoryItemEntry& ItemEntry)
{
	if (!IsValid(ItemEntry.Definition))
	{
		return;
	}

	UInventoryItemInstance* Instance = ItemEntry.GetInstance();
	if (Instance == nullptr)
	{
		// Create a new instance for this item entry if we should
		const UInventoryItemInstance* CDO = GetDefault<UInventoryItemInstance>(); //@TODO: Get CDO from definition
		if (ShouldCreateNewInstanceOfItem(ItemEntry) && !CDO->GetIsReplicated())
		{
			ItemEntry.LocalInstance = CreateNewInstanceOfItem(ItemEntry);
			Instance = ItemEntry.LocalInstance;

			// The instance should now be valid
			if (ensure(Instance))
			{
				//Instance->OnAddedToInventory()
			}
		}
	}

	// @TODO: Wrap this in a NotifyItemAdded() function
	// Broadcast the change event
	FInventoryChangeMessage Payload;
	{
		Payload.Controller = GetInstigatorController();
		Payload.Owner = GetOwner();
		Payload.Inventory = this;
		Payload.ItemEntry = &ItemEntry;
		Payload.NewStackCount = ItemEntry.StackCount;
		Payload.Delta = ItemEntry.StackCount;
	}

	OnItemAddedDelegate.Broadcast(Payload);

	ITEMIZATION_N_DISPLAY("Gave item [%s][%s] %s. Stack Count: %d (Max: %d), Source: %s",
			*ItemEntry.ItemHandle.ToString(),
			*GetNameSafe(ItemEntry.ReplInstance),
			*GetNameSafe(ItemEntry.Definition),
			ItemEntry.StackCount,
			ItemEntry.StackCount,
			*GetNameSafe(ItemEntry.SourceObject.Get()));
}

void AInventory::EvaluateItemEntry(
	const FInventoryItemEntry& ItemEntry,
	FInventoryTransaction_GiveItem& InOutTransaction)
{
	if (!ensure(ItemEntry.Definition))
	{
		ITEMIZATION_ERROR("Attempting to evaluate an item entry with no definition.");
		return;
	}

	// Evaluate the item entry
	FInventoryItemEntry& MutableItemEntry = const_cast<FInventoryItemEntry&>(ItemEntry);

	// Make sure the delta is in sync
	if (!ensure(ItemEntry.StackCount == InOutTransaction.Delta))
	{
		InOutTransaction.Delta = ItemEntry.StackCount;
	}

	// Iterate over all item components
	// @TODO: Profile this??
	for (const FItemComponentData* Component : ItemEntry.Definition->GetAllItemComponents())
	{
		Component->EvaluateItemEntry(MutableItemEntry, InOutTransaction);
	}
}

void AInventory::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Fast Arrays don't use the push model, but there is no harm in marking them with it.
	// The flag will just be ignored.
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_None;

	// If this is a player-controlled inventory, we should only replicate to the owner.
	if (InventoryType == EItemizationInventoryType::Player)
	{
		Params.Condition = COND_ReplayOrOwner;
	}

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryList, Params);

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool AInventory::ReplicateSubobjects(
	UActorChannel* Channel,
	FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	check(Channel);
	check(Bunch);
	check(RepFlags);
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// Register the item instances
	for (UInventoryItemInstance* Instance : GetAllItemInstances_Mutable())
	{
		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

bool AInventory::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

void AInventory::OnRep_InventoryList()
{
}

void AInventory::AddReplicatedItemInstance(UInventoryItemInstance* ItemInstance)
{
	TArray<TObjectPtr<UInventoryItemInstance>>& ReplicatedInstances = GetAllItemInstances_Mutable();
	if (ReplicatedInstances.Find(ItemInstance) == INDEX_NONE)
	{
		// Store it in the replicated list to avoid GC
		ReplicatedInstances.Add(ItemInstance);

		// Add it to the replicated sub object list if we're replicating
		if (IsUsingRegisteredSubObjectList())
		{
			AddReplicatedSubObject(ItemInstance, COND_None);
		}
	}
}

void AInventory::RemoveReplicatedItemInstance(UInventoryItemInstance* ItemInstance)
{
	const bool bWasRemoved = GetAllItemInstances_Mutable().RemoveSingle(ItemInstance) > 0;

	// Remove it from the replicated sub object list if we're replicating
	if (bWasRemoved && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

void AInventory::MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange)
{
	if (Owner->HasAuthority())
	{
		if (ItemEntry.ReplInstance == nullptr || bWasAddOrChange)
		{
			InventoryList.MarkItemDirty(ItemEntry);
		}
		else
		{
			// If the instance is invalid, and we're not adding or changing, we need to replicate the entire array
			InventoryList.MarkArrayDirty();
		}
	}
	else
	{
		// Client-side, mark the entire array dirty so it will be replicated
		InventoryList.MarkArrayDirty();
	}
}
