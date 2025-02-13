// Copyright Epic Games, Inc. All Rights Reserved.


#include "ActorComponents/EquipmentManager.h"

#include "ItemizationCoreLog.h"
#include "GameFramework/PlayerState.h"
#include "Misc/UObjectToken.h"
#include "Net/UnrealNetwork.h"

#include "InventoryEquipmentInstance.h"
#include "InventoryItemInstance.h"
#include "ItemDefinition.h"

#include "ItemizationCoreStats.h"
#include "ActorComponents/InventoryManager.h"
#include "Engine/ActorChannel.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentManager)

using namespace Itemization;

UEquipmentManager::UEquipmentManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;

	bWantsInventoryData = true;

	SetIsReplicatedByDefault(true);
}

UEquipmentManager* UEquipmentManager::GetEquipmentManager(AActor* Actor)
{
	UEquipmentManager* Mgr = nullptr;
	if (Actor)
	{
		Mgr = Actor->GetComponentByClass<UEquipmentManager>();
	}

	if (Mgr == nullptr)
	{
		if (const AController* C = Cast<AController>(Actor))
		{
			Mgr = C->GetPawn()->GetComponentByClass<UEquipmentManager>();
		}
		else if (const APlayerState* PS = Cast<APlayerState>(Actor))
		{
			Mgr = PS->GetPawn()->GetComponentByClass<UEquipmentManager>();
		}
	}

	return Mgr;
}

FInventoryItemEntryHandle UEquipmentManager::EquipItem(const FInventoryItemEntryHandle& ItemHandle, const FItemActionContextData& ContextData)
{
	if (!ItemHandle.IsValid())
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to equip an item with an invalid handle."),
			__FUNCTION__, *GetName());
		return FInventoryItemEntryHandle::NullHandle;
	}

	if (!IsOwnerActorAuthoritative())
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to equip item (%s) on the client-side."),
			__FUNCTION__, *GetName(), *ItemHandle.ToString());
		return FInventoryItemEntryHandle::NullHandle;
	}

	FInventoryItemEntry* ItemEntry = FindItemEntryFromHandle(ItemHandle);
	if (ItemEntry == nullptr)
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to equip an item that doesn't exist in the inventory."),
			__FUNCTION__, *GetName());
		return FInventoryItemEntryHandle::NullHandle;
	}

	if (ItemEntry->Instance != ContextData.Instigator.Get())
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to equip an item that doesn't belong to the instigator. Desired Instigator: (%s), Actual Instigator: (%s)"),
			__FUNCTION__, *GetName(), *GetNameSafe(ContextData.Instigator.Get()), *GetNameSafe(ItemEntry->Instance));
		return FInventoryItemEntryHandle::NullHandle;
	}

	if (FindEquipmentEntryFromHandle(ItemHandle) != nullptr)
	{
		ITEMIZATION_LOG(Warning, TEXT("[%hs] (%s): Attempted to equip an item that is already equipped."),
			__FUNCTION__, *GetName());
		return FInventoryItemEntryHandle::NullHandle;
	}

	APawn* Pawn = GetPawn();
	UInventoryManager* MutableInventoryManager = GetInventoryManager();

	FItemActionContextData CurrentContext = ContextData;
	MutableInventoryManager->EvaluateCurrentContext(*ItemEntry, CurrentContext);

	const FInventoryEquipmentEntry DefaultedEntry(ItemHandle);
	FInventoryEquipmentEntry& NewEquipment = EquipmentList.Items.Add_GetRef(DefaultedEntry);
	NewEquipment.SourceObject = ContextData.Instigator.Get();

	
	ITEMIZATION_LOG(Display, TEXT("[%hs] (%s): Equipping Item [%s] %s."),
		__FUNCTION__, *GetName(), *ItemHandle.ToString(), *GetNameSafe(ItemEntry->Instance));
	
	OnEquipItem(NewEquipment);
	MarkEquipmentEntryDirty(NewEquipment, true);

	return NewEquipment.Handle;
}

FInventoryItemEntryHandle UEquipmentManager::EquipItem(const FInventoryItemEntryHandle& ItemHandle)
{
	// Temp: Create a non-evaluated context data. Will be evaluated in EquipItem().
	FItemActionContextData ContextData;
	ContextData.InventoryManager = GetInventoryManager();

	if (const FInventoryItemEntry* ItemEntry = FindItemEntryFromHandle(ItemHandle))
	{
		ContextData.Instigator = ItemEntry->Instance;
	}
	else
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to equip an item that doesn't exist in the inventory."),
			__FUNCTION__, *GetName());
		return FInventoryItemEntryHandle::NullHandle;
	}

	// Equip the item and return its handle.
	// Will run validation and authority checks.
	return EquipItem(ItemHandle, ContextData);
}

FInventoryItemEntryHandle UEquipmentManager::K2_EquipItem(FInventoryItemEntryHandle ItemHandle)
{
	// Temp: Create a non-evaluated context data. Will be evaluated in EquipItem().
	FItemActionContextData ContextData;
	ContextData.InventoryManager = GetInventoryManager();

	if (const FInventoryItemEntry* ItemEntry = FindItemEntryFromHandle(ItemHandle))
	{
		ContextData.Instigator = ItemEntry->Instance;
	}
	else
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to equip an item that doesn't exist in the inventory."),
			__FUNCTION__, *GetName());
		return FInventoryItemEntryHandle::NullHandle;
	}

	// Equip the item and return its handle.
	// Will run validation and authority checks.
	return EquipItem(ItemHandle, ContextData);
}

void UEquipmentManager::OnEquipItem(FInventoryEquipmentEntry& EquipmentEntry)
{
	if (!EquipmentEntry.Handle.IsValid())
	{
		return;
	}

	UInventoryEquipmentInstance* Instance = EquipmentEntry.Instance;
	if (Instance == nullptr)
	{
		// Create a new instance.
		EquipmentEntry.Instance = CreateNewInstanceOfEquipment(EquipmentEntry);
		Instance = EquipmentEntry.Instance;
	}

	ITEMIZATION_Net_LOG(Display, this, TEXT("Equipping Item [%s] '%s' data '%s'"),
		*EquipmentEntry.Handle.ToString(), *GetNameSafe(Instance), GetInventoryData() ? *GetInventoryData()->ToString() : TEXT("null"));

	if (ensure(Instance))
	{
		Instance->OnEquipped(EquipmentEntry, GetInventoryDataPtr());
	}
}

bool UEquipmentManager::UnequipItem(
	const FInventoryItemEntryHandle& ItemHandle, const FItemActionContextData& ContextData)
{
	if (!ItemHandle.IsValid())
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to unequip an item with an invalid handle."),
			__FUNCTION__, *GetName());
		return false;
	}

	if (!IsOwnerActorAuthoritative())
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to unequip item (%s) on the client-side."),
			__FUNCTION__, *GetName(), *ItemHandle.ToString());
		return false;
	}

	FInventoryEquipmentEntry* EquipmentEntry = FindEquipmentEntryFromHandle(ItemHandle);
	if (EquipmentEntry == nullptr)
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to unequip an item that isn't equipped."),
			__FUNCTION__, *GetName());
		return false;
	}

	APawn* Pawn = GetPawn();
	UInventoryManager* MutableInventoryManager = GetInventoryManager();

	FItemActionContextData CurrentContext = ContextData;

	if (const FInventoryItemEntry* ItemEntry = FindItemEntryFromHandle(ItemHandle))
	{
		MutableInventoryManager->EvaluateCurrentContext(*ItemEntry, CurrentContext);	
	}
	else
	{
		ensureMsgf(false,
			TEXT("(%s): Couldn't find item entry for handle %s. Will still continue un-equipping the item but some functionality might not work.")
			, *GetName(), *ItemHandle.ToString());
	}

	bool bDidUnequip = false;
	for (auto It = EquipmentList.Items.CreateIterator(); It; ++It)
	{
		FInventoryEquipmentEntry& EquipmentIt = *It;
		check(EquipmentIt.Instance);

		if (EquipmentIt.Handle != ItemHandle)
		{
			continue;
		}

		OnUnequipItem(EquipmentIt);
		bDidUnequip = true;

		It.RemoveCurrent();
		EquipmentList.MarkArrayDirty();
	}

	return bDidUnequip;
}

bool UEquipmentManager::UnequipItem(const FInventoryItemEntryHandle& ItemHandle)
{
	// Temp: Create a non-evaluated context data. Will be evaluated in UnequipItem().
	FItemActionContextData ContextData;
	ContextData.InventoryManager = GetInventoryManager();

	if (const FInventoryItemEntry* ItemEntry = FindItemEntryFromHandle(ItemHandle))
	{
		ContextData.Instigator = ItemEntry->Instance;
	}
	else
	{
		ITEMIZATION_LOG(Error, TEXT("[%hs] (%s): Attempted to unequip an item that doesn't exist in the inventory."),
			__FUNCTION__, *GetName());
		return false;
	}

	// Unequip the item and return its handle.
	// Will run validation and authority checks.
	return UnequipItem(ItemHandle, ContextData);
}

void UEquipmentManager::OnUnequipItem(FInventoryEquipmentEntry& EquipmentEntry)
{
	if (!EquipmentEntry.Handle.IsValid())
	{
		return;
	}

	// Notify the equipment instance that it has been unequipped.
	if (UInventoryEquipmentInstance* Instance = EquipmentEntry.Instance)
	{
		Instance->OnUnequipped(EquipmentEntry, Instance->CurrentInventoryData);

		// Make sure we remove the instances before marking it as garbage.
		if (GetOwnerRole() == ROLE_Authority)
		{
			// Only destroy if we're authoritative.
			// Can't destroy on clients or replication will fail since the item will be marked as pending kill.
			RemoveReplicatedInstance(EquipmentEntry.SourceObject);
			RemoveReplicatedInstance(Instance);
		}

		Instance->MarkAsGarbage();
	}

	EquipmentEntry.Instance = nullptr;
}

void UEquipmentManager::MarkEquipmentEntryDirty(FInventoryEquipmentEntry& EquipmentEntry, bool bWasAddOrChange)
{
	if (IsOwnerActorAuthoritative())
	{
		if (EquipmentEntry.Instance == nullptr || bWasAddOrChange)
		{
			EquipmentList.MarkItemDirty(EquipmentEntry);
		}
		else
		{
			EquipmentList.MarkArrayDirty();
		}
	}
	else
	{
		// Client-side, mark the array dirty so it will be replicated.
		EquipmentList.MarkArrayDirty();
	}
}

UInventoryEquipmentInstance* UEquipmentManager::CreateNewInstanceOfEquipment(FInventoryEquipmentEntry& EquipmentEntry)
{
	check(EquipmentEntry.Instance == nullptr);
	
	const FInventoryItemEntry* ItemEntry = FindItemEntryFromHandle(EquipmentEntry.Handle);
	check(ItemEntry);

	// Gather the definition and instance.
	UItemDefinition* Definition = ItemEntry->Definition;
	check(Definition);
	UInventoryItemInstance* ItemInstance = ItemEntry->Instance;
	check(ItemInstance);

	// Create a new instance of the equipment, storing it in the equipment entry.
	UInventoryEquipmentInstance* NewInstance = NewObject<UInventoryEquipmentInstance>(ItemInstance, Definition->GetDefaultEquipmentInstanceClass());
	check(NewInstance);
	EquipmentEntry.Definition = Definition;
	EquipmentEntry.Instance = NewInstance;

	AddReplicatedInstance(NewInstance); // The equipment instance should be replicated.
	AddReplicatedInstance(ItemInstance); // The item instance as well, as other clients might need to know about it.
	return NewInstance;
}

void UEquipmentManager::AddReplicatedInstance(UObject* Instance)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(Instance);
	}
}

void UEquipmentManager::RemoveReplicatedInstance(UObject* Instance)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		RemoveReplicatedSubObject(Instance);
	}
}


FInventoryItemEntry* UEquipmentManager::FindItemEntryFromHandle(FInventoryItemEntryHandle Handle, EConsiderPending ConsiderPending) const
{
	if (!Handle.IsValid())
	{
		return nullptr;
	}

	UInventoryManager* MutableInventoryManager = GetInventoryManager();
	if (MutableInventoryManager == nullptr)
	{
		return nullptr;
	}

	return MutableInventoryManager->FindItemEntryFromHandle(Handle, ConsiderPending);
}

FInventoryEquipmentEntry* UEquipmentManager::FindEquipmentEntryFromHandle(FInventoryItemEntryHandle Handle) const
{
	SCOPE_CYCLE_COUNTER(STAT_FindEquipmentEntryFromHandle);

	for (const FInventoryEquipmentEntry& EquipmentEntry : EquipmentList.Items)
	{
		if (EquipmentEntry.Handle != Handle)
		{
			continue;
		}

		return const_cast<FInventoryEquipmentEntry*>(&EquipmentEntry);
	}

	return nullptr;
}

UInventoryEquipmentInstance* UEquipmentManager::GetFirstInstanceOfType(
	TSubclassOf<UInventoryEquipmentInstance> InstanceType) const
{
	for (auto& Entry : EquipmentList.Items)
	{
		if (UInventoryEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

void UEquipmentManager::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn())
	{
		ITEMIZATION_LOG(Error, TEXT("Component [%s] must be attached to a pawn."), *GetName());

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("EquipmentManager", "NotOnPawnError",
				"EquipmentManager has been added to a blueprint that is not a pawn. This component must be attached to a pawn.");
			static const FName LogName = TEXT("EquipmentManager");

			FMessageLog(LogName).Error()
			->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
			->AddToken(FTextToken::Create(Message));

			FMessageLog(LogName).Open();
		}
#endif
		return;
	}

	EquipmentList.RegisterWithOwner(this);
}

bool UEquipmentManager::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
#if SUBOBJECT_TRANSITION_VALIDATION
	if (UActorChannel::CanIgnoreDeprecatedReplicateSubObjects())
	{
		return false;
	}
#endif
	
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (const FInventoryEquipmentEntry& EquipmentEntry : EquipmentList.Items)
	{
		UInventoryEquipmentInstance* Instance = EquipmentEntry.Instance;

		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UEquipmentManager::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (const FInventoryEquipmentEntry& EquipmentEntry : EquipmentList.Items)
		{
			UInventoryEquipmentInstance* Instance = EquipmentEntry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

void UEquipmentManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentList);
}

void UEquipmentManager::GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const
{
	Super::GetReplicatedCustomConditionState(OutActiveState);
}

void UEquipmentManager::PreInitializeWithInventoryManager(UInventoryManager* InInventoryManager)
{
	check(InInventoryManager);
	InInventoryManager->SetAvatarActor(GetPawn());
	InInventoryManager->ForceAvatarReplication();
}
