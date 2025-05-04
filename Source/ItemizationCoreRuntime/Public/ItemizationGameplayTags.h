// Author: Tom Werner (MajorT), 2025

#pragma once

#include "NativeGameplayTags.h"

#define ITEMIZATION_TAG(TagName) ITEMIZATIONCORERUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TagName)

namespace Itemization::Tags
{
	/** Item Stats */
	ITEMIZATION_TAG(TAG_ItemStat)
	ITEMIZATION_TAG(TAG_ItemStat_MaxStackSize)
	ITEMIZATION_TAG(TAG_ItemStat_CurrentStackSize)
	
	/** Item Traits */
	ITEMIZATION_TAG(TAG_ItemTrait)
	ITEMIZATION_TAG(TAG_ItemTrait_AllowEmptyFinalStack)
	ITEMIZATION_TAG(TAG_ItemTrait_AllowEmptyStack)
	ITEMIZATION_TAG(TAG_ItemTrait_AllowItemSyncShare)
	ITEMIZATION_TAG(TAG_ItemTrait_AllowQuickbarFocusForGameplayOnly)
	ITEMIZATION_TAG(TAG_ItemTrait_AllowSwapSingleStack)
	ITEMIZATION_TAG(TAG_ItemTrait_AlwaysCountForCollectionQuest)
	ITEMIZATION_TAG(TAG_ItemTrait_AutoCombineStacks)
	ITEMIZATION_TAG(TAG_ItemTrait_CacheFiringRateOnWeaponFire)
	ITEMIZATION_TAG(TAG_ItemTrait_DisallowActivateOnQuickbarFocus)
	ITEMIZATION_TAG(TAG_ItemTrait_DisallowDepositInStorageVault)
	ITEMIZATION_TAG(TAG_ItemTrait_DisallowQuickbarFocus)
	ITEMIZATION_TAG(TAG_ItemTrait_ForceIntoOverflow)
	ITEMIZATION_TAG(TAG_ItemTrait_ForceQuickbarFocusWhenAdded)
	ITEMIZATION_TAG(TAG_ItemTrait_ForceStayInOverflow)
	ITEMIZATION_TAG(TAG_ItemTrait_HasDurability)
	ITEMIZATION_TAG(TAG_ItemTrait_HideItemToast)
	ITEMIZATION_TAG(TAG_ItemTrait_IgnoreRemoveAllInventoryItems)
	ITEMIZATION_TAG(TAG_ItemTrait_InventorySizeLimited)
	ITEMIZATION_TAG(TAG_ItemTrait_ShuffleTile)
	ITEMIZATION_TAG(TAG_ItemTrait_SingleStack)
	ITEMIZATION_TAG(TAG_ItemTrait_Transient)
}
