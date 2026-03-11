// Author: Tom Werner (dc: majort), 2026


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

void FItemComponentData::PostItemEntryCreated(
	FInventoryItemEntry& ItemEntry) const
{
	// Stub
}

bool FItemComponentData::CanMergeItems(
	const FInventoryItemEntry& ThisEntry,
	const FInventoryItemEntry& OtherEntry) const
{
	return true;
}

void FItemComponentData::OnItemGiven(
	FInventoryItemEntry& ItemEntry,
	AInventoryBase* Inventory) const
{
	// Stub
}

void FItemComponentData::OnItemRemoved(
	FInventoryItemEntry& ItemEntry,
	AInventoryBase* Inventory) const
{
	// Stub
}

#if WITH_EDITOR
EDataValidationResult FItemComponentData::IsDataValid(FDataValidationContext& Context) const
{
	return EDataValidationResult::Valid;
}
#endif
