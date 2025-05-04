// Author: Tom Werner (MajorT), 2025


#include "ItemizationGameplayTags.h"

namespace Itemization::Tags
{
	// ----------------------------------------------------------------------------------------------------------------
	//
	//			Item Stats
	//
	// ----------------------------------------------------------------------------------------------------------------

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemStat,
		"Item.Stat",
		"Parent tag for all item stats.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemStat_MaxStackSize,
		"Item.Stat.MaxStackSize",
		"Tag that is mapped to an integer that defines the maximum stack size of an item.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemStat_CurrentStackSize,
		"Item.Stat.CurrentStackSize",
		"Tag that is mapped to an integer that defines the current stack size of an item.");
	
	
	// ----------------------------------------------------------------------------------------------------------------
	//
	//			Item Traits
	//
	// ----------------------------------------------------------------------------------------------------------------
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait,
		"Item.Trait",
		"Parent tag for all item traits.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_AllowEmptyFinalStack,
		"Item.Trait.AllowEmptyFinalStack",
		"Trait for an item that whose final stack won't be removed from the inventory if it is empty. All other stacks will be removed when empty.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_AllowEmptyStack,
		"Item.Trait.AllowEmptyStack",
		"Trait for an item that won't be removed from the inventory when any of it's stacks is empty.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_AllowItemSyncShare,
		"Item.Trait.AllowItemSyncShare",
		"Trait for an item that can be shared across titles via itemSync Service.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_AllowQuickbarFocusForGameplayOnly,
		"Item.Trait.AllowQuickbarFocusForGameplayOnly",
		"Trait for an item that allows the quickbar to focus on the item only when using a gamepad, even if it's disallowed from the mouse-wheel.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_AllowSwapSingleStack,
		"Item.Trait.AllowSwapSingleStack",
		"Trait for an item that allows replacing the stack in the inventory with the one picked up or granted. Only used when the item has the SingleStack trait.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_AlwaysCountForCollectionQuest,
		"Item.Trait.AlwaysCountForCollectionQuest",
		"Trait for an item that will always be counted for collection quests.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_AutoCombineStacks,
		"Item.Trait.AutoCombineStacks",
		"Trait for an item that will automatically combine compatible stacks to fill larger stacks first.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_CacheFiringRateOnWeaponFire,
		"Item.Trait.CacheFiringRateOnWeaponFire",
		"Trait for an item to always use previous shooting's firing rate for checks, e.g. weapon still use ADS firing interval between previous shot ADS state and current shot in hip fire state.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_DisallowActivateOnQuickbarFocus,
		"Item.Trait.DisallowActivateOnQuickbarFocus",
		"Trait for an item that skips activation when focused in the quickbar.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_DisallowDepositInStorageVault,
		"Item.Trait.DisallowDepositInStorageVault",
		"Trait for an item that can't be deposited in storage vault.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_DisallowQuickbarFocus,
		"Item.Trait.DisallowQuickbarFocus",
		"Trait for an item that doesn't allow the quickbar to focus on the item.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_ForceIntoOverflow,
		"Item.Trait.ForceIntoOverflow",
		"Trait for an item that doesn't show up on any quickbar e.g. ability weapon items.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_ForceQuickbarFocusWhenAdded,
		"Item.Trait.ForceQuickbarFocusWhenAdded",
		"Trait for an item that gets quickbar focus when it is added to the inventory.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_ForceStayInOverflow,
		"Item.Trait.ForceStayInOverflow",
		"Trait for an item that if placed in overflow, will stay there.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_HasDurability,
		"Item.Trait.HasDurability",
		"Trait for an item that has durability.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_HideItemToast,
		"Item.Trait.HideItemToast",
		"Trait for an item that doesn't show a toast when the item gets added to the inventory");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_IgnoreRemoveAllInventoryItems,
		"Item.Trait.IgnoreRemoveAllInventoryItems",
		"Trait for an item that won't be removed during RemoveAllInventoryItems.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_InventorySizeLimited,
		"Item.Trait.InventorySizeLimited",
		"Trait for an item that should count towards the inventory limit.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_ShuffleTile,
		"Item.Trait.ShuffleTile",
		"Trait for an item that is a shuffle tile for random selections in the front end.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_SingleStack,
		"Item.Trait.SingleStack",
		"Trait for an item that can only have a single stack.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ItemTrait_Transient,
		"Item.Trait.Transient",
		"Trait for an item that should never be saved. Use for test items or items added at runtime.");
}
