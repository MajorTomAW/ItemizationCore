// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/ItemComponentData.h"

#include "Enums/EItemComponentInstancingPolicy.h"

FItemComponentData::FItemComponentData()
	: InstancingPolicy(EItemComponentInstancingPolicy::NonInstanced)
{
}

bool FItemComponentData::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	// Nothing to serialize in the base class
	bOutSuccess = true;
	return true;
}

void FItemComponentData::EvaluateItemEntry(
	FInventoryItemEntry& ItemEntry,
	const FInventoryItemTransactionBase& Transaction) const
{
	// Nothing to do here, can be overridden in derived classes
}
