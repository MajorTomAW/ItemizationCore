// Copyright © 2025 MajorT. All Rights Reserved.


#include "AssetDefinition_InventorySetupData.h"

#include "InventorySetupDataBase.h"

#define LOCTEXT_NAMESPACE "ItemizationCoreEditor"

FText UAssetDefinition_InventorySetupData::GetAssetDisplayName() const
{
	return LOCTEXT("AssetTypeInventorySetupData", "Inventory Setup Data");
}

FLinearColor UAssetDefinition_InventorySetupData::GetAssetColor() const
{
	return FLinearColor(FColor(201, 29, 85));
}

TSoftClassPtr<UObject> UAssetDefinition_InventorySetupData::GetAssetClass() const
{
	return UInventorySetupDataBase::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_InventorySetupData::GetAssetCategories() const
{
	static const auto Categories =
		{
			FAssetCategoryPath(FText::FromName("Itemization"))
		};
	
	return Categories;
}
