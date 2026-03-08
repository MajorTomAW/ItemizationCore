// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "InventorySlotId.h"
#include "SlottableInventoryComponent.generated.h"

#define UE_API ITEMIZATIONCORE_API

class ASlottableInventory;
class AInventoryBase;
struct FInventorySlotId;
struct FInventoryItemId;
struct FInventoryItemSlot;
struct FInventoryItemEntry;

/** Base class for all inventory managers that support a slottable inventory. */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent), MinimalAPI)
class USlottableInventoryComponent
	: public UInventoryComponent
{
	GENERATED_BODY()

public:
	UE_API USlottableInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin IGameplayTagAssetInterface Implementation
	UE_API virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~ End IGameplayTagAssetInterface Implementation

	/** Returns the owning slottable inventory. */
	UFUNCTION(BlueprintPure, Category=Inventory)
	UE_API ASlottableInventory* GetSlottableInventory() const;

	/**
	 * Attempts to place (and give) an item into a specified item slot and group.
	 * Will be ignored if the actor is not authoritative.
	 *
	 * @param ItemDefinition	The item definition to give/place.
	 * @param SlotId			The id of the slot to place the item in.
	 * @param StackSize		The number of items to give.
	 * @param SourceObject		Optional source object giving the item.
	 * @param GroupTag			Tag of the group to add the item to.
	 * @param OutNumCouldNotAdd [OUT] The amount of items that could not be added to the inventory due to stack size limit or other restrictions.
	 * @returns The id to the item that was added, or an invalid if if the item could not be added.
	 */
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintAuthorityOnly, meta=(Categories="Inventory.Group"), DisplayName="Place Item In Slot (Definition)")
	UE_API FInventoryItemId PlaceItemInSlot_Definition(const UItemDefinitionBase* ItemDefinition, const FInventorySlotId& SlotId, int32 StackSize, UObject* SourceObject, FGameplayTag GroupTag, int32& OutNumCouldNotAdd);

	UFUNCTION(BlueprintCallable, Category=Inventory, Server, Reliable, WithValidation)
	UE_API void Server_SwapItemSlots(FInventorySlotId SlotA, FGameplayTag SlotGroupA, FInventorySlotId SlotB, FGameplayTag SlotGroupB);

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintAuthorityOnly, meta=(Categories="Inventory.Group"))
	UE_API void SwapItemSlots(const FInventorySlotId& SlotA, FGameplayTag SlotGroupA, const FInventorySlotId& SlotB, FGameplayTag SlotGroupB);

	/** Tries to find an item slot id and its group tag by associated item. */
	UFUNCTION(BlueprintPure, Category=Inventory)
	UE_API FInventorySlotId FindSlotId(TScriptInterface<IInventoryItemInstanceInterface> ItemInstance, FGameplayTag& OutGroupTag) const;

	//~ Begin UObject Interface
#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

protected:
	//~ Begin UInventoryComponent Interface
	//UE_API virtual void OnInventoryCreated(AInventoryBase* Inventory) override;
	UE_API virtual void SetupInventory(AInventoryBase* Inventory) override;
	//~ End UInventoryComponent Interface

	UE_API virtual void InitInventoryGroups(AInventoryBase* Inventory);

public:
	/** Config data for this inventory manager. This should only be set through the editor as we currently don't support runtime changes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = InventoryConfig)
	TObjectPtr<class UInventoryConfigAsset> InventoryConfig;
};

#undef UE_API
