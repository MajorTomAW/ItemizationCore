// Author: Tom Werner (MajorT), 2025


#include "Inventory/InventoryBase.h"

#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

#include "Items/Data/ItemComponentData.h"
#include "Items/Data/ItemComponentData_Traits.h"

#include "Items/InventoryItemInstance.h"
#include "Items/ItemDefinitionBase.h"
#include "ItemizationCoreSettings.h"
#include "ItemizationGameplayTags.h"
#include "ItemizationLogChannels.h"
#include "Transactions/InventoryTransaction_GiveItem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryBase)

AInventoryBase::AInventoryBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bAlwaysRelevant = false;
	bNetLoadOnClient = true;
	bNetUseOwnerRelevancy = true;
	bReplicateUsingRegisteredSubObjectList = true;
	NetPriority = 3.0f;
	
	SetReplicatingMovement(false);
	SetNetUpdateFrequency(10.0f);
	SetNetCullDistanceSquared(112500000.0f);

	SetHidden(true);
	SetCanBeDamaged(false);
	bEnableAutoLODGeneration = false;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AInventoryBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Fast Arrays don't use the push model, but there is no harm in marking them with it.
	// The flag will just be ignored.
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_None;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryList, Params);
	
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool AInventoryBase::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
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

bool AInventoryBase::IsNetRelevantFor(
	const AActor* RealViewer,
	const AActor* ViewTarget,
	const FVector& SrcLocation) const
{
	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

void AInventoryBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

FInventoryItemHandle AInventoryBase::GiveItem(
	const FInventoryItemEntry& ItemEntry,
	int32& OutExcess,
	FInventoryTransaction_GiveItem& Transaction)
{
	check(ItemEntry.ItemDefinition);

	// If locked, add to the pending list

	// Evaluate the item entry
	EvaluateItemEntry(ItemEntry, Transaction);
	const int32 MaxStackSize = ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_MaxStackSize);
	

	ITEMIZATION_N_LOG("Giving item [%s] %s\tSize: %d/%d\tSource: %s",
		*ItemEntry.ItemHandle.ToString(),
		*GetNameSafe(ItemEntry.ItemDefinition),
		Transaction.Delta,
		MaxStackSize,
		*GetNameSafe(ItemEntry.SourceObject.Get()));
	ItemEntry.DebugPrintStats();

	return NativeGiveItem(ItemEntry, Transaction, OutExcess);
}

void AInventoryBase::EvaluateItemEntry(
	const FInventoryItemEntry& ItemEntry,
	FInventoryTransaction_GiveItem& InOutTransaction)
{
	if (!ensure(ItemEntry.ItemDefinition))
	{
		return;
	}

	if (!ensure(ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize)) ==
		InOutTransaction.Delta)
	{
		InOutTransaction.Delta = ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
	}

	FInventoryItemEntry& MutableEntry = const_cast<FInventoryItemEntry&>(ItemEntry);
	for (const FItemComponentData* ItemData : ItemEntry.ItemDefinition->GetDataList())
	{
		ItemData->EvaluateItemEntry(MutableEntry, InOutTransaction);
	}
}

FInventoryItemHandle AInventoryBase::NativeGiveItem(
	const FInventoryItemEntry& ItemEntry,
	FInventoryTransaction_GiveItem& Transaction,
	int32& OutExcess)
{
	OutExcess = Transaction.Delta;

	FInventoryItemHandle LastHandle;
	const int32 MaxStackSize = ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_MaxStackSize);

	// Try to fill up existing stacks first, before creating new ones
	if (MaxStackSize > 1)
	{
		for (FInventoryItemEntry& FoundEntry : InventoryList)
		{
			// Only consider items of the same type
			if (FoundEntry.ItemDefinition != ItemEntry.ItemDefinition)
			{
				continue;
			}

			const int32 OldStackSize = ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
			if (CanMergeItems(ItemEntry, FoundEntry))
			{
				// Merge the items
				int32 MergeExcess;
				MergeItems(ItemEntry, FoundEntry, MergeExcess);

				// Update the delta excess of what is left after the merge
				OutExcess = FMath::Max(0, MergeExcess);
				LastHandle = FoundEntry.ItemHandle;

				// Broadcast the change event
				NotifyItemChanged(FoundEntry, OldStackSize,
					FoundEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize));

				// Mark the item dirty for replication
				MarkItemEntryDirty(FoundEntry, true);
			}
		}
	}

	// If we have excess items, try to add them to the new inventory
	while (OutExcess > 0)
	{
		if (!CanCreateNewStack(ItemEntry, Transaction))
		{
			break;
		}

		// Subtract the new stack delta from the excess
		const int32 DeltaStack = FMath::Min(OutExcess, MaxStackSize);
		OutExcess -= DeltaStack;

		// Create a copy of the item entry and add it to the inventory
		FInventoryItemEntry EntryCopy = ItemEntry;
		EntryCopy.SetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize, DeltaStack);

		// Add it to the inventory and generate a new item uid
		FInventoryItemEntry& NewEntry = InventoryList.Items.Add_GetRef(EntryCopy);
		NewEntry.ItemHandle.GenerateNewUID();

		LastHandle = NewEntry.ItemHandle;

		// Create a mew instance server-side
		if (ShouldCreateNewInstanceOfItem(NewEntry))
		{
			CreateNewInstanceOfItem(NewEntry);
		}

		// Initialize
		OnGiveItem(NewEntry);

		// Mark dirty for replication
		MarkItemEntryDirty(NewEntry, true);
	}

	OutExcess = FMath::Max(0, OutExcess);
	return LastHandle;
}

bool AInventoryBase::CanMergeItems(
	const FInventoryItemEntry& ThisEntry,
	const FInventoryItemEntry& OtherEntry) const
{
	// Can't merge items if they're a different type
	if (ThisEntry.ItemDefinition != OtherEntry.ItemDefinition)
	{
		return false;
	}

	// Iterate over the data list and see if any of them are incompatible or restricted
	for (const FItemComponentData* ItemData : OtherEntry.ItemDefinition->GetDataList())
	{
		if (!ensure(ItemData))
		{
			continue;
		}

		if (!ItemData->CanMergeItems(ThisEntry, OtherEntry))
		{
			return false;
		}
	}
	return true;
}

void AInventoryBase::MergeItems(
	const FInventoryItemEntry& ThisEntry,
	FInventoryItemEntry& OtherEntry,
	int32& OutExcess) const
{
	// Gather max stack size
	const int32 MaxStackSize = ThisEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_MaxStackSize);

	// Calculate the excess number of items that couldn't be added to the base stack
	const int32 ThisStackSize = ThisEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
	const int32 OtherStackSize = OtherEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
	OutExcess = ThisStackSize + OtherStackSize - MaxStackSize;

	OtherEntry.SetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize,
		FMath::Min(MaxStackSize, ThisStackSize + OtherStackSize));
}

bool AInventoryBase::CanCreateNewStack(
	const FInventoryItemEntry& ItemEntry,
	const FInventoryTransaction_GiveItem& Transaction)
{
	if (!IsValid(ItemEntry.ItemDefinition))
	{
		return false;
	}

	// If we only allow a single stack of the item, only return true if we don't already have one
	if (FItemComponentData_Traits::HasTrait(ItemEntry.ItemDefinition,
		UItemizationCoreSettings::Get()->SingleStackTag))
	{
		return InventoryList.Items.Contains(ItemEntry.ItemDefinition) == false;
	}
	
	return false;
}

bool AInventoryBase::ShouldCreateNewInstanceOfItem(const FInventoryItemEntry& ItemEntry) const
{
	// Always return true, can be overridden in subclasses
	//@TODO: Maybe some items dont require having an instance??
	return true;
}

UInventoryItemInstance* AInventoryBase::CreateNewInstanceOfItem(FInventoryItemEntry& ItemEntry)
{
	checkf(ItemEntry.GetItemInstance() == nullptr, TEXT("Item instance already exists for item [%s]!"),
		*ItemEntry.GetDebugString());

	const UItemDefinitionBase* Definition = ItemEntry.ItemDefinition;
	check(Definition);

	//@TODO: Custom Instance Class
	const UClass* InstanceClass = UInventoryItemInstance::StaticClass();

	// Create the new instance
	UInventoryItemInstance* NewInstance = NewObject<UInventoryItemInstance>(this, InstanceClass);
	check(NewInstance);

	// Add it to our instances-list so that it doesn't get garbage collected
	if (NewInstance->GetIsReplicated())
	{
		AddReplicatedItemInstance(NewInstance);
		ItemEntry.SetReplicatedItemInstance(NewInstance);
	}
	else
	{
		ItemEntry.SetNonReplicatedItemInstance(NewInstance);
	}

	return NewInstance;
}

void AInventoryBase::OnRemoveItem(FInventoryItemEntry& ItemEntry)
{
	UInventoryItemInstance* Instance = ItemEntry.GetItemInstance();
	if (Instance == nullptr)
	{
		
	}

	// Broadcast the change event
	NotifyItemRemoved(ItemEntry, ItemEntry.LastObservedStackCount, 0);
}

void AInventoryBase::OnGiveItem(FInventoryItemEntry& ItemEntry)
{
	if (!IsValid(ItemEntry.ItemDefinition))
	{
		return;
	}

	UInventoryItemInstance* Instance = ItemEntry.GetItemInstance();
	if (Instance == nullptr)
	{
		// Create a new instance for this item entry in case we have a non-replicated one
		const UInventoryItemInstance* CDO = GetDefault<UInventoryItemInstance>();
		if (ShouldCreateNewInstanceOfItem(ItemEntry) && !CDO->GetIsReplicated())
		{
			Instance = CreateNewInstanceOfItem(ItemEntry);

			if (ensure(Instance))
			{
				//Instance->OnAddedToInventory() @TODO
			}
		}
	}
	
	// Broadcast the change event
	NotifyItemAdded(ItemEntry, ItemEntry.LastObservedStackCount,
		ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize));
}

void AInventoryBase::NotifyItemAdded(
	const FInventoryItemEntry& ItemEntry,
	const int32& LastCount,
	const int32& NewCount)
{
}

void AInventoryBase::NotifyItemRemoved(
	const FInventoryItemEntry& ItemEntry,
	const int32& LastCount,
	const int32& NewCount)
{
}

void AInventoryBase::NotifyItemChanged(
	const FInventoryItemEntry& ItemEntry,
	const int32& LastCount,
	const int32& NewCount)
{
}


void AInventoryBase::OnRep_InventoryList()
{
	for (FInventoryItemEntry& Entry : InventoryList)
	{
		if (!IsValid(Entry.GetItemInstance()))
		{
			// Queue up another call to make sure we get the item instance
			GetWorld()->GetTimerManager().SetTimer(OnRep_InventoryListTimerHandle, this, &ThisClass::OnRep_InventoryList, 0.5f);
			return;
		}
	}
}

void AInventoryBase::AddReplicatedItemInstance(UInventoryItemInstance* ItemInstance)
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

void AInventoryBase::RemoveReplicatedItemInstance(UInventoryItemInstance* ItemInstance)
{
	const bool bWasRemoved = GetAllItemInstances_Mutable().RemoveSingle(ItemInstance) > 0;

	// Remove it from the replicated sub object list if we're replicating
	if (bWasRemoved && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

void AInventoryBase::MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange)
{
	if (Owner->HasAuthority())
	{
		if (ItemEntry.GetItemInstance() == nullptr || bWasAddOrChange)
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
