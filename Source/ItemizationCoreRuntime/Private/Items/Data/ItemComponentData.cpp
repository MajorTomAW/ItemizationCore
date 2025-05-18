// Author: Tom Werner (MajorT), 2025


#include "Items/Data/ItemComponentData.h"

#include "UObject/AssetRegistryTagsContext.h"

FItemComponentData::FItemComponentData()
{
}

bool FItemComponentData::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	return true;
}

void FItemComponentData::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	// Stub
}

void FItemComponentData::EvaluateItemEntry(
	FInventoryItemEntry& ItemEntry,
	const FInventoryTrackableOp& Transaction) const
{
	// Stub
}

bool FItemComponentData::CanMergeItems(
	const FInventoryItemEntry& ThisEntry,
	const FInventoryItemEntry& OtherEntry) const
{
	return true;
}

void FItemComponentData::OnItemInstanceCreated(
	FInventoryItemEntry& ItemEntry,
	const FInventoryHandle& InventoryHandle) const
{
	// Stub
}

void FItemComponentData::OnItemInstanceRemoved(
	FInventoryItemEntry& ItemEntry,
	const FInventoryHandle& InventoryHandle) const
{
	// Stub
}

#if WITH_EDITOR
EDataValidationResult FItemComponentData::IsDataValid(FDataValidationContext& Context) const
{
	return EDataValidationResult::Valid;
}
#endif
