// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/ComponentData/ItemComponentData.h"

#include "Enums/EItemComponentInstancingPolicy.h"
#include "UObject/AssetRegistryTagsContext.h"

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

void FItemComponentData::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	// Nothing to do here. Can be overridden in derived classes
}

#if WITH_EDITOR
EDataValidationResult FItemComponentData::IsDataValid(FDataValidationContext& Context) const
{
	return EDataValidationResult::Valid;
}
#endif

void FItemComponentData::EvaluateItemEntry(
	FInventoryItemEntry& ItemEntry,
	const FInventoryItemTransactionBase& Transaction) const
{
	// Nothing to do here. Can be overridden in derived classes
}

bool FItemComponentData::CanMergeItems(
	const FInventoryItemEntry& ThisEntry,
	const FInventoryItemEntry& OtherEntry) const
{
	return true;
}
