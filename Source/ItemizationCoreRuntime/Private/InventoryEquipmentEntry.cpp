// Copyright Epic Games, Inc. All Rights Reserved.


#include "InventoryEquipmentEntry.h"

void FInventoryEquipmentEntry::PreReplicatedRemove(const FInventoryEquipmentContainer& InArraySerializer)
{
}

void FInventoryEquipmentEntry::PostReplicatedAdd(const FInventoryEquipmentContainer& InArraySerializer)
{
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
