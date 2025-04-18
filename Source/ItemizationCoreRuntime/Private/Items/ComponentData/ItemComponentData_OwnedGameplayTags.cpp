// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/ComponentData/ItemComponentData_OwnedGameplayTags.h"

#include "Items/InventoryItemEntry.h"
#include "UObject/AssetRegistryTagsContext.h"

FItemComponentData_OwnedGameplayTags::FItemComponentData_OwnedGameplayTags()
{
}

void FItemComponentData_OwnedGameplayTags::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	for (const FGameplayTag& Tag : Tags)
	{
		if (Tag.IsValid())
		{
			const UObject::FAssetRegistryTag RegTag =
				{
					FName("ItemTags"),
					Tag.GetTagName().ToString(),
					UObject::FAssetRegistryTag::TT_Alphabetical
				};
			
			Context.AddTag(RegTag);
		}
	}
}

void FItemComponentData_OwnedGameplayTags::EvaluateItemEntry(
	FInventoryItemEntry& ItemEntry,
	const FInventoryItemTransactionBase& Transaction) const
{
	ItemEntry.DynamicTags.AppendTags(Tags);
}
