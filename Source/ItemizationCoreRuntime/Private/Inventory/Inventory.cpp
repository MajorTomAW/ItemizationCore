// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/Inventory.h"

#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

#include "Enums/EItemizationInventoryType.h"
#include "Inventory/Transactions/InventoryItemTransactionBase.h"
#include "Items/ItemDefinition.h"
#include "InventoryItemHandle.h"
#include "InventorySlotHandle.h"
#include "ItemizationCoreSettings.h"
#include "ItemizationLogChannels.h"
#include "Inventory/InventoryDescriptor.h"
#include "Inventory/Messaging/InventoryChangeMessage.h"


#include "Items/ComponentData/ItemComponentData_MaxStackSize.h"
#include "Items/ComponentData/ItemComponentData_Traits.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Inventory)

AInventory::AInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
	, InventoryList(this)
{
	// By default, this inventory should be considered private and not replicated to other clients.
	bOnlyRelevantToOwner = true;
	bNetUseOwnerRelevancy = true;
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

FInventoryItemEntry* AInventory::FindItemEntryFromHandle(const FInventoryItemHandle& ItemHandle) const
{
	if (const FInventoryItemEntry* FoundEntry = InventoryList.Items.FindByKey(ItemHandle))
	{
		return const_cast<FInventoryItemEntry*>(FoundEntry);
	}

	return nullptr;
}

FInventoryItemHandle AInventory::NativeGiveItem(
	const FInventoryItemEntry& ItemEntry,
	const FInventoryTransaction_GiveItem& Transaction,
	int32& Excess)
{
	// Start by setting the excess to the delta
	Excess = Transaction.Delta;
	
	FInventoryItemHandle LastHandle;
	FInventoryTransaction_GiveItem& MutableTransaction = const_cast<FInventoryTransaction_GiveItem&>(Transaction);

	// Evaluate the max stack size. @TODO: Do we really have to manually search for this component data?
	const int32 MaxStackSize = FItemComponentData_MaxStackSize::GetMaxStackSize(ItemEntry.Definition);

	// Try to find existing stacks of the item and fill them up first, before creating new ones
	if (MaxStackSize > 1)
	{
		for (FInventoryItemEntry& FoundEntry : InventoryList)
		{
			// Only consider items of the same type
			if (FoundEntry.Definition != ItemEntry.Definition)
			{
				continue;
			}
			
			const int32 OldStackCount = ItemEntry.StackCount;
			if (CanMergeItems(ItemEntry, FoundEntry))
			{
				// Perform the merge action
				int32 LocalExcess;
				MergeItems(ItemEntry, FoundEntry, LocalExcess);

				// Update the delta excess of what is left after the merge
				Excess = FMath::Max(0, LocalExcess);
				LastHandle = FoundEntry.ItemHandle;

				// Broadcast the item change event
				NotifyItemChanged(FoundEntry, FoundEntry.LastObservedStackCount, FoundEntry.StackCount);

				// Mark the item dirty for replication
				MarkItemEntryDirty(FoundEntry, true);
			}
		}
	}

	// If we have excess items, try to add them to the inventory
	while (Excess > 0)
	{
		// Check if we can create a new stack for the item
		if (!CanCreateNewStack(ItemEntry, Transaction))
		{
			break;
		}

		// Create a new stack for the item
		// This also ensures that the new stack doesn't exceed the max stack size
		const int32 Delta = FMath::Min(Excess, MaxStackSize);
		Excess -= Delta;

		// Create a copy of the item entry that we can add to the inventory
		FInventoryItemEntry EntryCopy = ItemEntry;
		EntryCopy.StackCount = Delta;

		// Add the item to the inventory and generate a new item id
		FInventoryItemEntry& NewItem = InventoryList.Items.Add_GetRef(EntryCopy);
		NewItem.ItemHandle.GenerateNewUID();

		// Update the last handle to the one we just generated
		LastHandle = NewItem.ItemHandle;

		// Create a new instance server-side
		if (ShouldCreateNewInstanceOfItem(NewItem))
		{
			CreateNewInstanceOfItem(NewItem);
		}

		// Initialize the newly added item
		OnGiveItem(NewItem);

		// Mark dirty for replication
		MarkItemEntryDirty(NewItem, true);
	}

	Excess = FMath::Max(0, Excess);
	return LastHandle;
}

bool AInventory::CanMergeItems(
	const FInventoryItemEntry& ThisEntry,
	const FInventoryItemEntry& OtherEntry) const
{
	// Can't merge items if they're a different type
	if (ThisEntry.Definition != OtherEntry.Definition)
	{
		return false;
	}

	// Check for restrictions from the item components
	for (const FItemComponentData* ComponentData : OtherEntry.Definition->GetAllItemComponents())
	{
		if (!ensure(ComponentData) || !ComponentData->CanMergeItems(ThisEntry, OtherEntry))
		{
			return false;
		}
	}

	return true;
}

void AInventory::MergeItems(
	const FInventoryItemEntry& ThisEntry,
	FInventoryItemEntry& OtherEntry,
	int32& OutExcess) const
{
	// Gather max stack size
	const int32 MaxStackSize = FItemComponentData_MaxStackSize::GetMaxStackSize(ThisEntry.Definition);

	// Calculate the excess number of items that couldn't be added to the base stack
	OutExcess = ThisEntry.StackCount + OtherEntry.StackCount - MaxStackSize;
	OtherEntry.StackCount = FMath::Min(MaxStackSize, ThisEntry.StackCount + OtherEntry.StackCount);
}

bool AInventory::CanCreateNewStack(
	const FInventoryItemEntry& ItemEntry,
	const FInventoryTransaction_GiveItem& Transaction)
{
	// We can't create a new stack for an item with an invalid definition
	if (!IsValid(ItemEntry.Definition))
	{
		return false;
	}

	// Check for restrictions from the item tags
	if (FItemComponentData_Traits::HasTrait(ItemEntry.Definition, UItemizationCoreSettings::Get()->SingleStackTag))
	{
		// Check if we already have a stack of this item
		return InventoryList.Items.Contains(ItemEntry.Definition) == false;	
	}

	return true;
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
		ItemEntry.NonReplInstance = NewInstance;
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
	NotifyItemRemoved(ItemEntry, ItemEntry.LastObservedStackCount, 0);
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
			ItemEntry.NonReplInstance = CreateNewInstanceOfItem(ItemEntry);
			Instance = ItemEntry.NonReplInstance;

			// The instance should now be valid
			if (ensure(Instance))
			{
				//Instance->OnAddedToInventory()
			}
		}
	}
	
	// Broadcast the change event
	NotifyItemAdded(ItemEntry, 0, ItemEntry.StackCount);

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

void AInventory::NotifyItemAdded(
	const FInventoryItemEntry& ItemEntry,
	const int32& LastCount,
	const int32& NewCount)
{
	FInventoryChangeMessage Payload;
	{
		Payload.Controller = GetInstigatorController();
		Payload.Owner = GetOwner();
		Payload.Inventory = this;
		Payload.ItemEntry = &ItemEntry;
		Payload.NewStackCount = NewCount;
		Payload.Delta = NewCount - LastCount;
	}

	OnItemAddedDelegate.Broadcast(Payload);
}

void AInventory::NotifyItemRemoved(
	const FInventoryItemEntry& ItemEntry,
	const int32& LastCount,
	const int32& NewCount)
{
	FInventoryChangeMessage Payload;
	{
		Payload.Controller = GetInstigatorController();
		Payload.Owner = GetOwner();
		Payload.Inventory = this;
		Payload.ItemEntry = &ItemEntry;
		Payload.NewStackCount = NewCount;
		Payload.Delta = NewCount - LastCount;
	}

	OnItemRemovedDelegate.Broadcast(Payload);
}

void AInventory::NotifyItemChanged(
	const FInventoryItemEntry& ItemEntry,
	const int32& LastCount,
	const int32& NewCount)
{
	FInventoryChangeMessage Payload;
	{
		Payload.Controller = GetInstigatorController();
		Payload.Owner = GetOwner();
		Payload.Inventory = this;
		Payload.ItemEntry = &ItemEntry;
		Payload.NewStackCount = NewCount;
		Payload.Delta = NewCount - LastCount;
	}

	OnItemChangedDelegate.Broadcast(Payload);
}


void AInventory::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Fast Arrays don't use the push model, but there is no harm in marking them with it.
	// The flag will just be ignored.
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_None;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryOwner, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryAvatar, Params);

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

void AInventory::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Ensure an inventory owner
	if (!ensure(InventoryList.OwningInventory))
	{
		InventoryList.OwningInventory = this;
	}

	const UWorld* World = GetWorld();

	// Alloc the inventory descriptor if we don't have one
	if (!InventoryDescriptor.IsValid())
	{
		InventoryDescriptor = MakeShareable(new FInventoryDescriptorData);
	}

	// Default init the inventory descriptor to our outer owner
	AActor* MyOwner = GetOwner();
	InitInventoryDescriptor(MyOwner, MyOwner);
}

void AInventory::InitInventoryDescriptor(AActor* InOwner, AActor* InAvatar)
{
	check(InventoryDescriptor.IsValid());
	const bool bWasAvatarNull = InventoryDescriptor->InventoryAvatar == nullptr;
	const bool bAvatarChanged = InAvatar != InventoryDescriptor->InventoryAvatar.Get();

	InventoryDescriptor->InitFromActor(InOwner, InAvatar, this);
	SetInventoryOwner(InOwner);

	// Caching previous avatar to check against
	AActor* OldAvatar = GetInventoryAvatar_Direct();
	SetInventoryAvatar_Direct(InAvatar);

	// Notify all item instances about the avatar change
	if (bAvatarChanged)
	{
		//@TODO: ScopeLock
		for (const FInventoryItemEntry& ItemEntry : InventoryList)
		{
			if (IsValid(ItemEntry.GetInstance()))
			{
				//ItemEntry.GetInstance()->OnAvataSet @TODO
			}
		}
	}
}

void AInventory::ClearInventoryDescriptor()
{
	check(InventoryDescriptor.IsValid());
	InventoryDescriptor->Reset();
	SetInventoryOwner(nullptr);
	SetInventoryAvatar_Direct(nullptr);
}


void AInventory::OnRep_InventoryList()
{
}

void AInventory::OnRep_InventoryOwner()
{
	check(InventoryDescriptor.IsValid());

	AActor* OldOwner = GetInventoryOwner();
	AActor* OldAvatar = GetInventoryAvatar_Direct();

	if ((OldOwner != InventoryDescriptor->InventoryOwner) ||
		(OldOwner != InventoryDescriptor->InventoryAvatar))
	{
		if (OldOwner != nullptr)
		{
			InitInventoryDescriptor(OldOwner, OldAvatar);
		}
		else
		{
			ClearInventoryDescriptor();
		}
	}
}

void AInventory::SetInventoryOwner(AActor* NewOwnerActor)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InventoryOwner, this);

	if (InventoryOwner)
	{
		
	}

	InventoryOwner = NewOwnerActor;

	if (InventoryOwner)
	{
		
	}
}

void AInventory::SetInventoryAvatar_Direct(AActor* NewAvatarActor)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InventoryAvatar, this);

	if (InventoryAvatar)
	{
		
	}

	InventoryAvatar = NewAvatarActor;

	if (InventoryAvatar)
	{
		
	}
}

void AInventory::SetInventoryAvatar(AActor* NewAvatarActor)
{
	check(InventoryDescriptor.IsValid());
	InitInventoryDescriptor(GetInventoryOwner(), NewAvatarActor);
}

AActor* AInventory::GetInventoryAvatar() const
{
	check(InventoryDescriptor.IsValid());
	return InventoryDescriptor->InventoryAvatar.Get();
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
