// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationCoreTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait, "ItemizationCore.Item.Trait", "Parent tag for all item traits.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_AllowEmptyFinalStack, "ItemizationCore.Item.Trait.AllowEmptyFinalStack", "Trait for an item that whose final stack won't be removed from the inventory if it is empty. All other stacks will be removed when empty.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_AllowEmptyStack, "ItemizationCore.Item.Trait.AllowEmptyStack", "Trait for an item that won't be removed from the inventory when any of it's stacks is empty.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_AllowItemSyncShare, "ItemizationCore.Item.Trait.AllowItemSyncShare", "Trait for an item that can be shared across titles via itemSync Service.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_AllowQuickbarFocusForGameplayOnly, "ItemizationCore.Item.Trait.AllowQuickbarFocusForGameplayOnly", "Trait for an item that allows the quickbar to focus on the item only when using a gamepad, even if it's disallowed from the mouse-wheel.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_AllowSwapSingleStack, "ItemizationCore.Item.Trait.AllowSwapSingleStack", "Trait for an item that allows replacing the stack in the inventory with the one picked up or granted. Only used when the item has the SingleStack trait.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_AlwaysCountForCollectionQuest, "ItemizationCore.Item.Trait.AlwaysCountForCollectionQuest", "Trait for an item that will always be counted for collection quests.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_AutoCombineStacks, "ItemizationCore.Item.Trait.AutoCombineStacks", "Trait for an item that will automatically combine compatible stacks to fill larger stacks first.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_CacheFiringRateOnWeaponFire, "ItemizationCore.Item.Trait.CacheFiringRateOnWeaponFire", "Trait for an item to always use previous shooting's firing rate for checks, e.g. weapon still use ADS firing interval between previous shot ADS state and current shot in hip fire state.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_DisallowActivateOnQuickbarFocus, "ItemizationCore.Item.Trait.DisallowActivateOnQuickbarFocus", "Trait for an item that skips activation when focused in the quickbar.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_DisallowDepositInStorageVault, "ItemizationCore.Item.Trait.DisallowDepositInStorageVault", "Trait for an item that can't be deposited in storage vault.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_DisallowQuickbarFocus, "ItemizationCore.Item.Trait.DisallowQuickbarFocus", "Trait for an item that doesn't allow the quickbar to focus on the item.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_ForceIntoOverflow, "ItemizationCore.Item.Trait.ForceIntoOverflow", "Trait for an item that doesn't show up on any quickbar e.g. ability weapon items.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_ForceQuickbarFocusWhenAdded, "ItemizationCore.Item.Trait.ForceQuickbarFocusWhenAdded", "Trait for an item that gets quickbar focus when it is added to the inventory.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_ForceStayInOverflow, "ItemizationCore.Item.Trait.ForceStayInOverflow", "Trait for an item that if placed in overflow, will stay there.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_HasDurability, "ItemizationCore.Item.Trait.HasDurability", "Trait for an item that has durability.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_HideItemToast, "ItemizationCore.Item.Trait.HideItemToast", "Trait for an item that doesn't show a toast when the item gets added to the inventory");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_IgnoreRemoveAllInventoryItems, "ItemizationCore.Item.Trait.IgnoreRemoveAllInventoryItems", "Trait for an item that won't be removed during RemoveAllInventoryItems.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_InventorySizeLimited, "ItemizationCore.Item.Trait.InventorySizeLimited", "Trait for an item that should count towards the inventory limit.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_ShuffleTile, "ItemizationCore.Item.Trait.ShuffleTile", "Trait for an item that is a shuffle tile for random selections in the front end.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_SingleStack, "ItemizationCore.Item.Trait.SingleStack", "Trait for an item that can only have a single stack.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemizationCoreTags::TAG_ItemTrait_Transient, "ItemizationCore.Item.Trait.Transient", "Trait for an item that should never be saved. Use for test items or items added at runtime.");