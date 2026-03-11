// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryBase.h"
#include "InventorySlotId.h"
#include "ItemInstanceBase.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "InventoryLibrary.generated.h"

#define UE_API ITEMIZATIONCORE_API

class UItemDefinitionBase;
class IInventoryOwnerInterface;
class UItemInstanceBase;
class AInventoryBase;
/** Library with a lot of useful helper functions */
UCLASS(MinimalAPI)
class UInventoryLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Creates a new item slot id. */
	UFUNCTION(BlueprintPure, Category=Inventory, meta=(NativeMakeFunc))
	static UE_API FInventorySlotId MakeInventorySlotId(int64 Row, int64 Column);

	/** Extracts data from an InventorySlotId. */
	UFUNCTION(BlueprintPure, Category=Inventory, meta=(NativeBreakFunc))
	static UE_API void BreakInventorySlotId(const FInventorySlotId& SlotId, int64& Row, int64& Column);

	/** Creates a new item id. */
	UFUNCTION(BlueprintPure, Category=Inventory, meta=(NativeMakeFunc))
	static UE_API FInventoryItemId MakeInventoryItemId(int64 Id);

	/** Extracts data from an InventoryItemId. */
	UFUNCTION(BlueprintPure, Category=Inventory, meta=(NativeBreakFunc))
	static UE_API void BreakInventoryItemId(const FInventoryItemId& ItemId, int64& Id);






	/** Tries to find an inventory by searching for the InventoryOwner interface on the actor or its components. */
	UFUNCTION(BlueprintPure, Category=Inventory, meta=(DefaultToSelf="Actor"))
	static UE_API AInventoryBase* FindInventory(const AActor* Actor);

	/** Returns the item instance associated to the given item id. */
	UFUNCTION(BlueprintPure, Category=Inventory)
	static UE_API UItemInstanceBase* GetItem(const AInventoryBase* Inventory, const FInventoryItemId& ItemId);

	/** Returns the item instance inside the given slot. */
	UFUNCTION(BlueprintPure, Category=Inventory, meta=(Categories="Inventory.Group"))
	static UE_API UItemInstanceBase* GetItemInSlot(const AInventoryBase* Inventory, const FInventorySlotId& SlotId, FGameplayTag GroupTag);

	/** Returns true if the item definition has a "TraitsItemData" which contains the given trait. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Itemization|Item", meta=(Categories="Item.Trait"))
	static UE_API bool ItemHasTrait(const UItemDefinitionBase* ItemDefinition, FGameplayTag TraitToCheck);

	/** Returns true if the item definition can be stacked multiple times in a single slot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Itemization|Item")
	static UE_API bool IsItemStackable(const UItemDefinitionBase* ItemDefinition);

	/** Tries to find the slot id and group of an item. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Itemization|Item")
	static UE_API FInventorySlotId FindSlotId(AInventoryBase* Inventory, const UItemInstanceBase* ItemInstance, FGameplayTag& OutGroupTag);


	/** Gives an item to an inventory owner. */
	UFUNCTION(BlueprintCallable, Category=Inventory, meta=(DefaultToSelf="InventoryOwner", Categories="Inventory.Group"), BlueprintAuthorityOnly)
	static UE_API UItemInstanceBase* GiveItemToInventoryOwner(const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner, const UItemDefinitionBase* ItemDefinition, int32 NumToGive, UObject* SourceObject, FGameplayTag PreferredGroup, int32& OutNumCouldNotGive);

	UFUNCTION(BlueprintCallable, Category=Inventory, meta=(DefaultToSelf="InventoryOwner", Categories="Inventory.Group"))
	static UE_API void Server_GiveItemToInventoryOwner(const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner, const UItemDefinitionBase* ItemDefinition, int32 NumToGive, UObject* SourceObject, FGameplayTag PreferredGroup);

	/** Gives an item to an inventory owner and places it in the specified slot. */
	UFUNCTION(BlueprintCallable, Category=Inventory, meta=(DefaultToSelf="InventoryOwner"), BlueprintAuthorityOnly, meta=(Categories="Inventory.Group"))
	static UE_API UItemInstanceBase* PlaceItemInSlotForInventoryOwner(const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner, const UItemDefinitionBase* ItemDefinition, int32 NumToGive, UObject* SourceObject, const FInventorySlotId& SlotId, FGameplayTag GroupTag, int32& OutNumCouldNotGive);

	/** Removes an item from the inventory. */
	UFUNCTION(BlueprintCallable, Category=Inventory, meta=(DefaultToSelf="InventoryOwner"), BlueprintAuthorityOnly)
	static UE_API int32 RemoveItemFromInventoryOwner(const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner, const UItemInstanceBase* Item, int32 NumToRemove = -1);

	/** Swaps the items in the two specified slots. */
	UFUNCTION(BlueprintCallable, Category=Inventory, meta=(DefaultToSelf="InventoryOwner", Categories="Inventory.Group"), BlueprintAuthorityOnly)
	static UE_API void SwapItemSlotsOnInventoryOwner(const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner, const FInventorySlotId& SlotA, FGameplayTag GroupTagA, const FInventorySlotId& SlotB, FGameplayTag GroupTagB);

	/** Performs a server rpc to call SwapItemSlots. */
	UFUNCTION(BlueprintCallable, Category=Inventory, meta=(DefaultToSelf="InventoryOwner", Categories="Inventory.Group"))
	static UE_API void Server_SwapItemSlotsOnInventoryOwner(const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner, const FInventorySlotId& SlotA, FGameplayTag GroupTagA, const FInventorySlotId& SlotB, FGameplayTag GroupTagB);

	/** Performs a server rpc to call DropItem. */
	UFUNCTION(BlueprintCallable, Category=Inventory, meta=(DefaultToSelf="InventoryOwner"))
	static UE_API void Server_DropItem(const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner, const FInventoryItemId& ItemId, int32 NumToDrop = -1);
};

#undef UE_API
