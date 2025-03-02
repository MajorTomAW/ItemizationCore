// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemEntry.h"

#include "ItemizationCoreLog.h"
#include "InventoryItemInstance.h"
#include "ItemDefinition.h"
#include "ActorComponents/InventoryManager.h"
#include "Components/ItemComponentData_MaxStackSize.h"

#if WITH_GAMEPLAY_MESSAGE_ROUTER
#include "GameFramework/GameplayMessageSubsystem.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Inventory_Message_ItemChange, "ItemizationCore.Inventory.Message.ItemChange");
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryItemEntry)


FInventoryItemEntry::FInventoryItemEntry()
	: Instance(nullptr)
	, StackCount(0)
	, LastObservedStackCount(INDEX_NONE)
	, bPendingRemove(false)
{
	Handle.GenerateNewHandle();
}

FInventoryItemEntry::FInventoryItemEntry(UItemDefinition* ItemDefinition, int32 InStackCount, UObject* InSourceObject)
: Definition(ItemDefinition)
	, StackCount(InStackCount)
	, LastObservedStackCount(StackCount)
	, SourceObject(InSourceObject)
	, bPendingRemove(false)
{
	Handle.GenerateNewHandle();
}

bool FInventoryItemEntry::IsStackFull() const
{
	if (Definition == nullptr)
	{
		return true;
	}

	if (const FItemComponentData_MaxStackSize* MaxStackSize = Definition->GetItemComponent<FItemComponentData_MaxStackSize>())
	{
		if (MaxStackSize->MaxStackSize.Value <= 0.f)
		{
			return false;
		}

		return StackCount >= MaxStackSize->MaxStackSize.Value;
	}

	return true;
}

void FInventoryItemEntry::PreReplicatedRemove(const struct FInventoryItemContainer& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		UE_LOG(LogInventorySystem, Verbose, TEXT("%s: OnRemoveItem (Non-Auth): [%s] %s. Stack Count: %d"), *GetNameSafe(InArraySerializer.Owner->GetOwner()), *Handle.ToString(), *GetNameSafe(Instance), StackCount);

		FScopedInventoryListLock InventoryListLock(*InArraySerializer.Owner);
		InArraySerializer.Owner->OnRemoveItem(*this);
	}
}

void FInventoryItemEntry::PostReplicatedAdd(const struct FInventoryItemContainer& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		UE_LOG(LogInventorySystem, Display, TEXT("%s: OnAddItem (Non-Auth): [%s] %s. Stack Count: %d"), *GetNameSafe(InArraySerializer.Owner->GetOwner()), *Handle.ToString(), *GetNameSafe(Instance), StackCount);

		InArraySerializer.Owner->OnGiveItem(*this);
	}
	else
	{
		ITEMIZATION_LOG(Warning, TEXT("PostReplicatedAdd called with no owner."));
	}
}

void FInventoryItemContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Index : RemovedIndices)
	{
		FInventoryItemEntry& ItemEntry = Items[Index];
		Owner->GetOnItemRemoved().Broadcast(FInventoryChangeMessage(ItemEntry.Instance, ItemEntry.StackCount, 0));
#if WITH_GAMEPLAY_MESSAGE_ROUTER
		BroadcastInventoryChangeMessage(ItemEntry.Instance, ItemEntry.StackCount, 0);
#endif
		ItemEntry.LastObservedStackCount = 0;
	}
}

void FInventoryItemContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FInventoryItemEntry& ItemEntry = Items[Index];
		Owner->GetOnItemAdded().Broadcast(FInventoryChangeMessage(ItemEntry.Instance, 0, ItemEntry.StackCount));
#if WITH_GAMEPLAY_MESSAGE_ROUTER
		BroadcastInventoryChangeMessage(ItemEntry.Instance, 0, ItemEntry.StackCount);
#endif
		ItemEntry.LastObservedStackCount = ItemEntry.StackCount;
	}
}

void FInventoryItemContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Index : ChangedIndices)
	{
		FInventoryItemEntry& ItemEntry = Items[Index];
		check(ItemEntry.LastObservedStackCount != INDEX_NONE);
		Owner->GetOnItemChanged().Broadcast(FInventoryChangeMessage(ItemEntry.Instance, ItemEntry.LastObservedStackCount, ItemEntry.StackCount));
#if WITH_GAMEPLAY_MESSAGE_ROUTER
		BroadcastInventoryChangeMessage(ItemEntry.Instance, ItemEntry.LastObservedStackCount, ItemEntry.StackCount);
#endif
		ItemEntry.LastObservedStackCount = ItemEntry.StackCount;
	}
}

void FInventoryItemContainer::RegisterWithOwner(UInventoryManager* InOwner)
{
	Owner = InOwner;
}

void FInventoryItemContainer::GetItemsOfType(
	TArray<FInventoryItemEntry*>* OutItems,
	const UItemDefinition* ItemDefinitionType,
	bool bIncompleteStacksOnly)
{
	if (OutItems == nullptr)
	{
		return;
	}

	OutItems->Reset();

	if (ItemDefinitionType == nullptr)
	{
		return;
	}

	for (FInventoryItemEntry& ItemEntry : Items)
	{
		if (ItemEntry.Definition == nullptr)
		{
			continue;
		}

		if (ItemEntry.Definition != ItemDefinitionType)
		{
			continue;
		}

		if (ItemEntry.bPendingRemove)
		{
			continue;
		}

		if (ItemEntry.IsStackFull() && bIncompleteStacksOnly)
		{
			continue;
		}

		OutItems->Add(&ItemEntry);
	}
}

#if WITH_GAMEPLAY_MESSAGE_ROUTER
void FInventoryItemContainer::BroadcastInventoryChangeMessage(UInventoryItemInstance* ItemThatChanged, const int32 OldStackCount, const int32 NewStackCount) const
{
	const FInventoryChangeMessage Message = FInventoryChangeMessage(ItemThatChanged, OldStackCount, NewStackCount);

	UGameplayMessageSubsystem& MessageSub = UGameplayMessageSubsystem::Get(Owner->GetWorld());
	MessageSub.BroadcastMessage(TAG_Inventory_Message_ItemChange, Message);
}
#endif

FScopedInventoryListLock::FScopedInventoryListLock(UInventoryManager& InManager)
	: InventoryManager(InManager)
{
	InventoryManager.IncrementInventoryListLock();
}

FScopedInventoryListLock::~FScopedInventoryListLock()
{
	InventoryManager.DecrementInventoryListLock();
}
