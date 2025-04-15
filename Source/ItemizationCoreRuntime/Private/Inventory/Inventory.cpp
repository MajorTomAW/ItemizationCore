// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/Inventory.h"

#include "ItemizationLogChannels.h"
#include "Engine/ActorChannel.h"
#include "Enums/EItemizationInventoryType.h"
#include "Inventory/Transactions/InventoryItemTransactionBase.h"
#include "Items/ItemComponentData.h"
#include "Items/ItemDefinition.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Inventory)

AInventory::AInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
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
		*ItemEntry.Handle.ToString(),
		*GetNameSafe(ItemEntry.Definition),
		Transaction.Delta,
		Transaction.Delta,
		*GetNameSafe(ItemEntry.SourceObject.Get()));

	return NativeGiveItem(ItemEntry, Transaction, OutExcess);
}

FInventoryItemHandle AInventory::NativeGiveItem(
	const FInventoryItemEntry& ItemEntry,
	const FInventoryTransaction_GiveItem& Transaction,
	int32& OutExcess)
{
	FInventoryItemHandle LastHandle;
	FInventoryTransaction_GiveItem& MutableTransaction = const_cast<FInventoryTransaction_GiveItem&>(Transaction);

	// Start by setting the excess to the delta
	OutExcess = Transaction.Delta;



	return LastHandle;
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
	const bool bWasRemoved = GetAllItemInstances_Mutable().RemoveSingle(ItemInstance);

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
		if (ItemEntry.Instance == nullptr || bWasAddOrChange)
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
