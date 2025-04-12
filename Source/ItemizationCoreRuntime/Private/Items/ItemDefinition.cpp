// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/ItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemDefinition)

#define LOCTEXT_NAMESPACE "ItemDefinition"

UItemDefinition::UItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FPrimaryAssetId UItemDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(GetFName(), PrimaryAssetType.AssetTypeId);
}

#undef LOCTEXT_NAMESPACE