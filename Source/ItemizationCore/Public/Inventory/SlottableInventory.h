// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "InventoryBase.h"
#include "Operations/InventoryOp_PlaceItemInSlot.h"

#include "SlottableInventory.generated.h"

#define UE_API ITEMIZATIONCORE_API

/**
 * Inventory class that manages an inventory list.
 * Other than AInventoryBase, this inventory also has the concept of slots and slot groups.
 * Adding support for things like: inventory, quick bar, perks, etc.
 *
 * These item slots still store the FInventoryItemEntry, however, they only act as a representation of the actual
 * FInventoryItemEntry which still lives inside the FInventoryItemList.
 */
UCLASS(MinimalAPI)
class ASlottableInventory : public AInventoryBase
{
	GENERATED_BODY()
	friend struct FInventoryItemSlot;

public:
	UE_API ASlottableInventory(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Attempts to place an item into an inventory slot.
	 * This is an advanced function, consider calling the wrappers on UInventoryComponent instead.
	 * 
	 * @param Params Cached params about the place item in slot action,
	 * @returns A pointer to the inventory operation. Can be used to get return data.
	 */
	UE_API virtual TInventoryOpPtr<FInventoryOp_PlaceItemInSlot> PlaceItemInSlot(FInventoryOp_PlaceItemInSlot::FParams&& Params);

	/** Returns the next unoccupied item slot in the given inventory group. If no group tag is set, will search every single group. */
	UFUNCTION(BlueprintPure, Category=Inventory, meta=(Categories="Inventory.Group"))
	FInventorySlotId GetNextUnoccupiedSlotIdInGroup(FGameplayTag GroupTag) const;
	FInventoryItemSlot* GetNextUnoccupiedSlotInGroup(const FGameplayTag& GroupTag) const;

protected:
	//~ Begin UObject Interface
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UObject Interface

	/**
	 * Marks a slot entry dirty for replication.
	 * bWasAddOrChange is an important flag to determine whether the entire array needs to be replicated,
	 * or if we can just replicate the slot delta entry.
	 */
	UE_API void MarkItemSlotDirty(FInventoryItemSlot& ItemSlot, bool bWasAddOrChange = false, bool bForceMarkSlotDirty = false);

	/** Processes a place item in slot operation which is ensured to be valid. */
	UE_API virtual void ProcessPlaceItemInSlotOperation(const TInventoryOpRef<FInventoryOp_PlaceItemInSlot>& PlaceItemInSlotOp);

protected:
	/** Replicated list of inventory slots. */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated, Category=Inventory)
	FInventorySlotList InventorySlotList;
};

#undef UE_API