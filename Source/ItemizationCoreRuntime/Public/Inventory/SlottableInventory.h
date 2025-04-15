// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryBase.h"
#include "Slots/InventorySlotEntry.h"

#include "SlottableInventory.generated.h"

/**
 * Class to manage the slots of an inventory.
 * On a low level, a single FInventorySlotEntry doesn't store the item itself, rather a handle to it.
 *
 * These slots are primarily used to manage and organize items in an inventory (grid).
 * By default, slots aren't limited to any specific item type and have to be configured via the item traits.
 *
 * @see FInventorySlotEntry, FInventoryItemHandle
 */
UCLASS(BlueprintType, Blueprintable, NotPlaceable)
class ASlottableInventory : public AInventoryBase
{
	GENERATED_BODY()
	friend class AInventory;

public:
	ASlottableInventory(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UObject Interface

	//~ Begin AInventoryBase Interface
	virtual void PostInitInventory() override;
	//~ End AInventoryBase Interface

protected:
	/** Whether the inventory slots should be replicated to other clients. */
	UPROPERTY(EditDefaultsOnly, Category=Inventory, AdvancedDisplay)
	uint8 bReplicateSlotsToClients : 1;

	/** The minimum number of slots to reserve in the inventory. */
	UPROPERTY(EditDefaultsOnly, Category=Inventory)
	uint16 MinSlots = 1;
	
	/** Replicated inventory slot list. */
	UPROPERTY(ReplicatedUsing = OnRep_InventorySlots, Transient, BlueprintReadOnly, Category=Inventory)
	FInventorySlotContainer InventorySlots;

	/** OnRep function for InventorySlots. */
	UFUNCTION()
	virtual void OnRep_InventorySlots();
};
