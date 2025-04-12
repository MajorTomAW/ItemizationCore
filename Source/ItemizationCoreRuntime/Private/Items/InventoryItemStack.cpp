// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/InventoryItemStack.h"

#include "Items/InventoryItemInstance.h"

FInventoryItemStack::FInventoryItemStack()
{
}

bool FInventoryItemStack::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;

	UObject* Obj = Instance.Get();
	Map->SerializeObject(Ar, Obj->GetClass(), Obj);

	return true;
}

bool FInventoryItemStack::operator==(const FInventoryItemStack& Other) const
{
	return Instance == Other.Instance;
}
