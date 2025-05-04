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
}

void FItemComponentData::EvaluateItemEntry(
	FInventoryItemEntry& ItemEntry,
	const FInventoryItemTransactionBase& Transaction) const
{
}

bool FItemComponentData::CanMergeItems(
	const FInventoryItemEntry& ThisEntry,
	const FInventoryItemEntry& OtherEntry) const
{
	return true;
}

#if WITH_EDITOR
EDataValidationResult FItemComponentData::IsDataValid(FDataValidationContext& Context) const
{
	return EDataValidationResult::Valid;
}
#endif
