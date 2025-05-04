// Author: Tom Werner (MajorT), 2025


#include "ItemizationCoreSettings.h"

#include "ItemizationGameplayTags.h"

UItemizationCoreSettings::UItemizationCoreSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AllowEmptyStackTag = Itemization::Tags::TAG_ItemTrait_AllowEmptyStack;
	HideToastTag = Itemization::Tags::TAG_ItemTrait_HideItemToast;
	IgnoreRemoveAllTag = Itemization::Tags::TAG_ItemTrait_IgnoreRemoveAllInventoryItems;
	CountTowardsLimitTag = Itemization::Tags::TAG_ItemTrait_InventorySizeLimited;
	SingleStackTag = Itemization::Tags::TAG_ItemTrait_SingleStack;
	TransientTag = Itemization::Tags::TAG_ItemTrait_Transient;
}

const UItemizationCoreSettings* UItemizationCoreSettings::Get()
{
	return GetDefault<UItemizationCoreSettings>();
}

UItemizationCoreSettings* UItemizationCoreSettings::GetMutable()
{
	return GetMutableDefault<UItemizationCoreSettings>();
}
