// Author: Tom Werner (dc: majort), 2026 January


#include "Items/Data/ItemComponentData_PickupBase.h"

#include "InventoryBase.h"


FItemComponentData_PickupBase::FItemComponentData_PickupBase()
{
}

FPickupCreationData FItemComponentData_PickupBase::GetPickupCreationData(
	const FInventoryItemEntry& ItemEntry,
	const AInventoryBase* Inventory) const
{
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

		FVector EyesLoc;
		FRotator EyesRot;
		Actor->GetActorEyesViewPoint(EyesLoc, EyesRot);
		Impl.Location = EyesLoc + Actor->GetActorForwardVector() * SpawnOffset;

		// Assign the data
		CreationData.Set(Impl);
	}


	return MoveTemp(CreationData);
}
