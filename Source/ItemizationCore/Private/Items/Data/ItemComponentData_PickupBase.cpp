// Author: Tom Werner (dc: majort), 2026 January


#include "Items/Data/ItemComponentData_PickupBase.h"

#include "InventoryHandle.h"
#include "Inventory/InventoryBase.h"


FItemComponentData_PickupBase::FItemComponentData_PickupBase()
{
}

FPickupCreationData FItemComponentData_PickupBase::GetPickupCreationData(
	const FInventoryItemEntry& ItemEntry,
	const FInventoryHandle& InventoryHandle) const
{
	AInventoryBase* Inventory = InventoryHandle.GetInventory();
	check(Inventory)

	FPickupCreationData CreationData;
	if (AActor* InventoryOwner = Inventory->GetOwner())
	{
		AActor* Actor = InventoryOwner;
		if (AController* C = Cast<AController>(InventoryOwner))
		{
			if (APawn* P = C->GetPawn())
			{
				Actor = P;
			}
		}

		FPickupCreationData_Impl Impl;
		Impl.PickupItemEntry = ItemEntry;
		Impl.OptionalInstigator = InventoryOwner;
		Impl.OverridePickupActorClass = PickupActorClass;
		Impl.Rotation = Actor->GetActorRotation();

		// By default, spawn 40 cm in front of the owner
		constexpr float SpawnOffset = 40.f;
		Impl.Location = Actor->GetActorLocation() + Actor->GetActorForwardVector() * SpawnOffset;

		// Assign the data
		CreationData.Set(Impl);
	}


	return MoveTemp(CreationData);
}
