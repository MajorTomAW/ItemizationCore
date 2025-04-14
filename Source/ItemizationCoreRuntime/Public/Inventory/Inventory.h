// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryBase.h"
#include "Items/InventoryItemEntry.h"

#include "Inventory.generated.h"

/**
 * The inventory class that holds a list of items.
 * Each item that lives in this class is purely a data object and doesn't have any physical representation.
 */
UCLASS(BlueprintType, Blueprintable, NotPlaceable)
class ITEMIZATIONCORERUNTIME_API AInventory : public AInventoryBase
{
	GENERATED_BODY()

public:
	AInventory(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Initializes the inventory with the given setup data. */
	virtual void GrantStartingItems(TArray<const FInventoryStartingItem*> StartingItems);

protected:
	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~ End UObject Interface

protected:
	/** Replicated list of inventory item entries. */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryList, BlueprintReadOnly, Category = Inventory)
	FInventoryItemContainer InventoryList;

	UFUNCTION()
	virtual void OnRep_InventoryList();
};
