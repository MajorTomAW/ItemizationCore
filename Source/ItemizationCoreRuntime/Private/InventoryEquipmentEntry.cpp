// Copyright Epic Games, Inc. All Rights Reserved.


#include "InventoryEquipmentEntry.h"

#include "InventoryEquipmentInstance.h"
#include "ItemizationCoreLog.h"
#include "ActorComponents/EquipmentManager.h"

FInventoryEquipmentEntry::FInventoryEquipmentEntry()
	: Instance(nullptr)
{
}

FInventoryEquipmentEntry::FInventoryEquipmentEntry(const FInventoryItemEntryHandle& InHandle)
	: Handle(InHandle)
	, Instance(nullptr)
{
}

void FInventoryEquipmentEntry::PreReplicatedRemove(const FInventoryEquipmentContainer& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		UE_LOG(LogInventorySystem, Verbose, TEXT("%s: OnRemoveEquipment (Non-Auth): [%s] %s."),
			*GetNameSafe(InArraySerializer.Owner->GetOwner()), *Handle.ToString(), *GetNameSafe(Instance));
		InArraySerializer.Owner->OnUnequipItem(*this);
	}
	else
	{
		ITEMIZATION_LOG(Warning, TEXT("PreReplicatedRemove called with no owner."));
	}
}

void FInventoryEquipmentEntry::PostReplicatedAdd(const FInventoryEquipmentContainer& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		UE_LOG(LogInventorySystem, Verbose, TEXT("%s: OnAddEquipment (Non-Auth): [%s] %s."),
			*GetNameSafe(InArraySerializer.Owner->GetOwner()), *Handle.ToString(), *GetNameSafe(Instance));
		InArraySerializer.Owner->OnEquipItem(*this);
	}
	else
	{
		ITEMIZATION_LOG(Warning, TEXT("PostReplicatedAdd called with no owner."));
	}
}

void FInventoryEquipmentEntry::PostReplicatedChange(const FInventoryEquipmentContainer& InArraySerializer)
{
}




void FInventoryEquipmentContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
}

void FInventoryEquipmentContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
}

void FInventoryEquipmentContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}

void FInventoryEquipmentContainer::RegisterWithOwner(UEquipmentManager* InOwner)
{
	Owner = InOwner;
}
