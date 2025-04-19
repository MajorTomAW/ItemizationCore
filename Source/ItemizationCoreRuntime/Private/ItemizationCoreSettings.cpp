// Copyright © 2025 MajorT. All Rights Reserved.


#include "ItemizationCoreSettings.h"

#include "ItemizationCoreTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemizationCoreSettings)

UItemizationCoreSettings::UItemizationCoreSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryName = TEXT("Game");
	SectionName = TEXT("Itemization Core");
	
	ItemTypes = {"Item", "Test"};

	AllowEmptyStackTag = ItemizationCoreTags::TAG_ItemTrait_AllowEmptyStack;
	HideToastTag = ItemizationCoreTags::TAG_ItemTrait_HideItemToast;
	IgnoreRemoveAllTag = ItemizationCoreTags::TAG_ItemTrait_IgnoreRemoveAllInventoryItems;
	CountTowardsLimitTag = ItemizationCoreTags::TAG_ItemTrait_InventorySizeLimited;
	SingleStackTag = ItemizationCoreTags::TAG_ItemTrait_SingleStack;
	TransientTag = ItemizationCoreTags::TAG_ItemTrait_Transient;
}

UItemizationCoreSettings* UItemizationCoreSettings::Get()
{
	return GetMutableDefault<UItemizationCoreSettings>();
}