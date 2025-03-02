// Copyright © 2024 Playton. All Rights Reserved.


#include "ActorComponents/InventorySlotManager.h"

#include "InventoryItemInstance.h"
#include "ItemizationCoreLog.h"
#include "ActorComponents/InventoryManager.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InventorySlotManager)

using namespace Itemization;

UInventorySlotManager::UInventorySlotManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SlotContainer(this)
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;

	bWantsInventoryData = true;

	SetIsReplicatedByDefault(true);
}

void UInventorySlotManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, SlotContainer);
}

UInventorySlotManager* UInventorySlotManager::FindInventorySlotManager(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	UInventorySlotManager* Mgr = nullptr;
	Mgr = Actor->FindComponentByClass<UInventorySlotManager>();

	if (Mgr == nullptr)
	{
		if (const APawn* P = Cast<APawn>(Actor))
		{
			Mgr = P->GetController() ? P->GetController()->FindComponentByClass<UInventorySlotManager>() : nullptr;
			if (Mgr == nullptr)
			{
				Mgr = P->GetPlayerState() ? P->GetPlayerState()->FindComponentByClass<UInventorySlotManager>() : nullptr;
			}
		}
		else if (const AController* C = Cast<AController>(Actor))
		{
			Mgr = C->GetPawn() ? C->GetPawn()->FindComponentByClass<UInventorySlotManager>() : nullptr;
			if (Mgr == nullptr)
			{
				Mgr = C->GetPlayerState<APlayerState>() ? C->GetPlayerState<APlayerState>()->FindComponentByClass<UInventorySlotManager>() : nullptr;
			}
		}
	}

	return Mgr;
}

int32 UInventorySlotManager::AddItemToSlot(const FInventoryItemEntryHandle& ItemHandle, int32 PreferredSlotId)
{
	// Attempted to add an invalid item handle
	if (!ItemHandle.IsValid())
	{
		return INDEX_NONE;
	}

	UInventoryManager* InventoryManager = GetInventoryManager();
	FInventoryItemEntry* FoundEntry = InventoryManager->FindItemEntryFromHandle(ItemHandle);
	if (!FoundEntry)
	{
		ITEMIZATION_Net_LOG(Error, this, TEXT("Failed to find item entry for handle %s"), *ItemHandle.ToString());
		return INDEX_NONE;
	}

	FInventorySlotEntry* FoundSlot = SlotContainer.Find(PreferredSlotId);
	if (FoundSlot == nullptr || FoundSlot->GetHandle().IsValid())
	{
		// Slot is already occupied
		// Find a new slot
		for (auto& Slot : SlotContainer.Slots)
		{
			if (Slot.IsValid())
			{
				continue;
			}

			PreferredSlotId = Slot.GetSlotID();
			FoundSlot = &Slot;
			break;
		}
	}
	if (FoundSlot == nullptr)
	{
		ITEMIZATION_Net_LOG(Error, this, TEXT("%hs Failed to find slot entry for slot id %d"), __FUNCTION__, PreferredSlotId);
		return INDEX_NONE;
	}

	// Don't add the item to the slot if it's pending remove
	if (FoundEntry->bPendingRemove)
	{
		return INDEX_NONE;
	}

	UInventoryItemInstance* Instance = FoundEntry->Instance;
	if (!IsValid(Instance))
	{
		ITEMIZATION_Net_LOG(Error, this, TEXT("Failed to find item instance for handle %s"), *ItemHandle.ToString());
		return INDEX_NONE;
	}

	const FItemizationCoreInventoryData* InventoryData = GetInventoryDataPtr();
	if (InventoryData == nullptr || !InventoryData->OwnerActor.IsValid() || !InventoryData->AvatarActor.IsValid())
	{
		ITEMIZATION_Net_LOG(Error, this, TEXT("Failed to find inventory data for handle %s"), *ItemHandle.ToString());
		return INDEX_NONE;
	}

	const ENetRole NetMode = InventoryData->AvatarActor->GetLocalRole();

	// This shouldn't get called from a simulated proxy
	if (NetMode == ROLE_SimulatedProxy)
	{
		return INDEX_NONE;
	}

	bool bIsLocallyControlled = InventoryData->IsLocallyControlled();


	// Perform a client-predictive add
	if (NetMode != ROLE_Authority)
	{
		if (true /*CanAddItemToSlot*/)
		{
			Server_AddItemToSlot(ItemHandle, PreferredSlotId);
			return PreferredSlotId;
		}
		else
		{
			
		}
	}
	
	return NativeAddItemToSlot(ItemHandle, PreferredSlotId);
}

void UInventorySlotManager::Server_AddItemToSlot_Implementation(
	const FInventoryItemEntryHandle& ItemHandle, int32 PreferredSlotId)
{
	if (ensure(ItemHandle.IsValid()))
	{
		NativeAddItemToSlot(ItemHandle, PreferredSlotId);
	}
	else
	{
		// Revert??
		if (FInventorySlotEntry* SlotEntry = SlotContainer.Find(PreferredSlotId))
		{
			SlotContainer.MarkItemDirty(*SlotEntry);
		}
	}
}

int32 UInventorySlotManager::NativeAddItemToSlot(const FInventoryItemEntryHandle& ItemHandle, int32 PreferredSlotId)
{
	if (!ItemHandle.IsValid())
	{
		ITEMIZATION_Net_LOG(Warning, this, TEXT("NativeAddItemToSlot called with an invalid item handle! SlotManager: %s. Avatar: %s"),
			*GetName(), *GetNameSafe(GetAvatarActor()));
		return INDEX_NONE;
	}

	UInventoryManager* InventoryManager = GetInventoryManager();
	FInventoryItemEntry* FoundEntry = InventoryManager->FindItemEntryFromHandle(ItemHandle);
	if (!FoundEntry)
	{
		ITEMIZATION_Net_LOG(Warning, this, TEXT("Failed to find item entry for handle %s"), *ItemHandle.ToString());
		return INDEX_NONE;
	}

	const FItemizationCoreInventoryData* InventoryData = GetInventoryDataPtr();
	if (InventoryData == nullptr || !InventoryData->OwnerActor.IsValid() || !InventoryData->AvatarActor.IsValid())
	{
		ITEMIZATION_Net_LOG(Error, this, TEXT("Failed to find inventory data for handle %s"), *ItemHandle.ToString());
		return INDEX_NONE;
	}

	// Again, this should only come from locally controlled actors or remote
	ENetRole NetMode = ROLE_SimulatedProxy;
	if (const AController* Controller = InventoryData->Controller.Get())
	{
		NetMode = Controller->GetLocalRole();
	}
	else if (const AActor* LocalAvatar = InventoryManager->GetAvatarActor_Direct())
	{
		NetMode = LocalAvatar->GetLocalRole();
	}

	if (NetMode == ROLE_SimulatedProxy)
	{
		return INDEX_NONE;
	}

	const bool bIsLocallyControlled = InventoryData->IsLocallyControlled();
	UInventoryItemInstance* Instance = FoundEntry->Instance;
	if (!IsValid(Instance))
	{
		ITEMIZATION_Net_LOG(Error, this, TEXT("Failed to find item instance for handle %s"), *ItemHandle.ToString());
		return INDEX_NONE;
	}

	FInventorySlotEntry* SlotEntry = nullptr;
	if (NetMode == ROLE_Authority)
	{
		if (!bIsLocallyControlled)
		{
			// Tell the client that the item was added to a slot
		}

		// Actually add the item to the slot
		SlotEntry = SlotContainer.Find(PreferredSlotId);
		if (ensure(SlotEntry))
		{
			SlotEntry->GetHandle_Ref() = ItemHandle;
		}
	}
	else
	{
		// Flush moves?
		if (!InventoryData->HasNetAuthority())
		{
		}

		// Send a server request to add the item to the slot
		// Server_...

		// Actually add the item to the slot
		SlotEntry = SlotContainer.Find(PreferredSlotId);
		if (ensure(SlotEntry))
		{
			SlotEntry->GetHandle_Ref() = ItemHandle;
		}
	}

	// Mark as dirty so we replicate
	if (IsOwnerActorAuthoritative() && SlotEntry)
	{
		SlotContainer.MarkItemDirty(*SlotEntry);
	}
	else
	{
		SlotContainer.MarkArrayDirty();
	}

	OnSlotEntryAdded.Broadcast(InventoryManager, this, *SlotEntry, SlotEntry->GetSlotID());
	ITEMIZATION_Net_LOG(Warning, this, TEXT("Item [%s] added to slot [%d]"), *ItemHandle.ToString(), SlotEntry->GetSlotID());
	
	return PreferredSlotId;
}

void UInventorySlotManager::RemoveItemFromSlot(const FInventoryItemEntryHandle& ItemHandle)
{
	// Attempted to add an invalid item handle
	if (!ItemHandle.IsValid())
	{
		ITEMIZATION_Net_LOG(Error, this, TEXT("Attempted to remove an item with an invalid handle [%s]"),
			*ItemHandle.ToString());
		return;
	}

	UInventoryManager* InventoryManager = GetInventoryManager();
	FInventoryItemEntry* FoundEntry = InventoryManager->FindItemEntryFromHandle(ItemHandle);
	if (!FoundEntry)
	{
		ITEMIZATION_Net_LOG(Warning, this, TEXT("Failed to find item entry for handle %s"), *ItemHandle.ToString());
		// No return here, we might have to clean up the slot
	}

	FInventorySlotEntry* FoundSlot = SlotContainer.Find(ItemHandle);
	if (FoundSlot == nullptr || !FoundSlot->IsValid())
	{
		ITEMIZATION_Net_LOG(Warning, this, TEXT("Failed to find slot entry for item handle %s OR the slot was already empty."), *ItemHandle.ToString());
		return;
	}

	const FItemizationCoreInventoryData* InventoryData = GetInventoryDataPtr();
	if (InventoryData == nullptr || !InventoryData->OwnerActor.IsValid() || !InventoryData->AvatarActor.IsValid())
	{
		ITEMIZATION_Net_LOG(Error, this, TEXT("Failed to find inventory data for handle %s"), *ItemHandle.ToString());
		return;
	}

	const ENetRole NetMode = InventoryData->AvatarActor->GetLocalRole();

	// This shouldn't get called from a simulated proxy
	if (NetMode == ROLE_SimulatedProxy)
	{
		ITEMIZATION_Net_LOG(Error, this, TEXT("RemoveItemFromSlot called from a simulated proxy! SlotManager: %s. Avatar: %s"),
			*GetName(), *GetNameSafe(GetAvatarActor()));
		return;
	}

	bool bIsLocallyControlled = InventoryData->IsLocallyControlled();

	// Perform a client-predictive add
	if (NetMode != ROLE_Authority)
	{
		Server_RemoveItemFromSlot(ItemHandle);
		return;
	}

	NativeRemoveItemFromSlot(ItemHandle);
}
void UInventorySlotManager::Server_RemoveItemFromSlot_Implementation(const FInventoryItemEntryHandle& ItemHandle)
{
	if (ensure(ItemHandle.IsValid()))
	{
		NativeRemoveItemFromSlot(ItemHandle);
	}
	else
	{
		// Revert??
		// But idk what to revert tho, lol
		if (FInventorySlotEntry* SlotEntry = SlotContainer.Find(ItemHandle))
		{
			SlotContainer.MarkItemDirty(*SlotEntry);
		}
	}
}
void UInventorySlotManager::NativeRemoveItemFromSlot(const FInventoryItemEntryHandle& ItemHandle)
{
	if (!ItemHandle.IsValid())
	{
		ITEMIZATION_Net_LOG(Warning, this, TEXT("NativeRemoveItemFromSlot called with an invalid item handle! SlotManager: %s. Avatar: %s"),
			*GetName(), *GetNameSafe(GetAvatarActor()));
		return;
	}

	UInventoryManager* InventoryManager = GetInventoryManager();
	FInventoryItemEntry* FoundEntry = InventoryManager->FindItemEntryFromHandle(ItemHandle);
	if (!FoundEntry)
	{
		ITEMIZATION_Net_LOG(Warning, this, TEXT("Failed to find item entry for handle %s"), *ItemHandle.ToString());
		// No return here, we might have to clean up the slot
	}

	const FItemizationCoreInventoryData* InventoryData = GetInventoryDataPtr();
	if (InventoryData == nullptr || !InventoryData->OwnerActor.IsValid() || !InventoryData->AvatarActor.IsValid())
	{
		ITEMIZATION_Net_LOG(Error, this, TEXT("Failed to find inventory data for handle %s"), *ItemHandle.ToString());
		return;
	}

	// Again, this should only come from locally controlled actors or remote
	ENetRole NetMode = ROLE_SimulatedProxy;
	if (const AController* Controller = InventoryData->Controller.Get())
	{
		NetMode = Controller->GetLocalRole();
	}
	else if (const AActor* LocalAvatar = InventoryManager->GetAvatarActor_Direct())
	{
		NetMode = LocalAvatar->GetLocalRole();
	}

	if (NetMode == ROLE_SimulatedProxy)
	{
		return;
	}

	const bool bIsLocallyControlled = InventoryData->IsLocallyControlled();
	FInventorySlotEntry* SlotEntry = nullptr;
	if (NetMode == ROLE_Authority)
	{
		if (!bIsLocallyControlled)
		{
			// Tell the client that the item was removed from the slot
		}

		// Actually remove the item from the slot
		SlotEntry = SlotContainer.Find(ItemHandle);
		if (ensure(SlotEntry))
		{
			SlotEntry->GetHandle_Ref().Clear();
		}
	}
	else
	{
		// Send a server request to remove the item from the slot
		// Server_...

		// Actually remove the item from the slot
		SlotEntry = SlotContainer.Find(ItemHandle);
		if (ensure(SlotEntry))
		{
			SlotEntry->GetHandle_Ref().Clear();
		}
		ITEMIZATION_Net_LOG(Warning, this, TEXT("NetMode is not ROLE_Authority, but we're removing the item from the slot anyway."));
	}

	// Mark as dirty so we replicate
	if (IsOwnerActorAuthoritative())
	{
		SlotContainer.MarkItemDirty(*SlotEntry);
	}
	else
	{
		SlotContainer.MarkArrayDirty();
	}

	OnSlotEntryRemoved.Broadcast(InventoryManager, this, *SlotEntry, SlotEntry->GetSlotID());
	ITEMIZATION_Net_LOG(Warning, this, TEXT("Item [%s] removed from slot [%d]"), *ItemHandle.ToString(), SlotEntry->GetSlotID());
}

void UInventorySlotManager::SwapSlots(int32 SlotIdA, int32 SlotIdB)
{
	// Don't swap if the slots are the same
	if (SlotIdA == SlotIdB)
	{
		ITEMIZATION_LOG(Warning, TEXT("Attempted to swap the same slot [%d] with itself."), SlotIdA);
		return;
	}

	// Validate both slots
	if (SlotIdA < 0 || SlotIdB < 0)
	{
		ITEMIZATION_LOG(Warning, TEXT("Attempted to swap invalid slots [%d] and [%d]."), SlotIdA, SlotIdB);
		return;
	}

	// Cache both slots
	FInventorySlotEntry* SlotA = FindSlotEntry(SlotIdA);
	FInventorySlotEntry* SlotB = FindSlotEntry(SlotIdB);

	// Now swap the slots
	if (ensure(SlotA) && ensure(SlotB))
	{
		SlotContainer.Swap(SlotA->GetSlotID(), SlotB->GetSlotID());

		// Mark both slots as dirty
        SlotContainer.MarkItemDirty(*SlotA);
		SlotContainer.MarkItemDirty(*SlotB);

		// Broadcast the change event for both slots
		OnSlotEntryChanged.Broadcast(GetInventoryManager(), this, *SlotA, SlotA->GetSlotID());
		OnSlotEntryChanged.Broadcast(GetInventoryManager(), this, *SlotB, SlotB->GetSlotID());
	}
}

int32 UInventorySlotManager::FindSlotIdByHandle(const FInventoryItemEntryHandle& Handle, bool& OutSuccess) const
{
	const FInventorySlotEntry* SlotEntry = SlotContainer.Find(Handle);
	OutSuccess = (SlotEntry != nullptr);
	if (OutSuccess)
	{
		return SlotEntry->GetSlotID();
	}
	
	return INDEX_NONE;
}

FInventoryItemEntryHandle UInventorySlotManager::FindHandleBySlotId(const int32 SlotId, bool& OutSuccess) const
{
	const FInventorySlotEntry* SlotEntry = SlotContainer.Find(SlotId);
	OutSuccess = (SlotEntry != nullptr);
	if (OutSuccess)
	{
		return SlotEntry->GetHandle();
	}
	
	return FInventoryItemEntryHandle::NullHandle;
}

void UInventorySlotManager::FindSlotById(int32 SlotId, bool& OutSuccess, FInventorySlotEntry& OutSlot) const
{
	const FInventorySlotEntry* SlotEntry = SlotContainer.Find(SlotId);
	OutSuccess = (SlotEntry != nullptr);
	if (OutSuccess)
	{
		OutSlot = *SlotEntry;
	}
}

UInventoryItemInstance* UInventorySlotManager::FindItemInstanceBySlotId(int32 SlotId) const
{
	if (const FInventorySlotEntry* SlotEntry = SlotContainer.Find(SlotId))
	{
		if (const UInventoryManager* InventoryManager = GetInventoryManager())
		{
			if (FInventoryItemEntry* ItemEntry = InventoryManager->FindItemEntryFromHandle(SlotEntry->GetHandle()))
			{
				return ItemEntry->Instance;
			}
		}
	}

	return nullptr;
}

const TArray<FInventorySlotEntry>& UInventorySlotManager::GetSlotEntries() const
{
	return SlotContainer.Slots;
}



const FInventorySlotEntry* UInventorySlotManager::FindSlotEntry(int32 SlotId) const
{
	return SlotContainer.Find(SlotId);
}

FInventorySlotEntry* UInventorySlotManager::FindSlotEntry(int32 SlotId)
{
	return SlotContainer.Find(SlotId);
}

void UInventorySlotManager::InitializeComponent()
{
	Super::InitializeComponent();

	// Make sure to only create slots on authority and in a game world
	// (PIE) included ofc
	if (HasAuthority() && GetWorld()->IsGameWorld())
	{
		auto GetTotalNumSlots = [this]() -> int32
		{
			int32 TotalSlots = 0;
			for (const auto& Constructor : DefaultSlots)
			{
				TotalSlots += Constructor.NumSlots;
			}
			return TotalSlots;
		};
		
		// Pre-allocate the slot entries
		SlotContainer.Slots.Reserve(GetTotalNumSlots());

		// Create the default slots
		for (const FInventorySlotConstructor& Constructor : DefaultSlots)
		{
			// Create the specified number of slots
			for (int32 i = 0; i < Constructor.NumSlots; ++i)
			{
				FInventorySlotEntry& SlotEntry = SlotContainer.Slots.AddDefaulted_GetRef();
				SlotEntry.GetSlotTag_Ref() = Constructor.SlotTag;
				SlotEntry.GetSlotID_Ref() = i;
				
				SlotContainer.MarkItemDirty(SlotEntry);

				ITEMIZATION_LOG(Display, TEXT("Slot [%d][%s] created for %s"), SlotEntry.GetSlotID(), *Constructor.SlotTag.ToString(), *GetName());
			}
		}
	}
}

bool UInventorySlotManager::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void UInventorySlotManager::ReadyForReplication()
{
	Super::ReadyForReplication();
}

void UInventorySlotManager::GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const
{
	Super::GetReplicatedCustomConditionState(OutActiveState);
}