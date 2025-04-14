// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryBase.h"
#include "Items/InventoryItemEntry.h"

#include "EquippableInventory.generated.h"

/**
 * The equippable inventory class.
 * Holds a list of items that count as equippable.
 * These items will be replicated to every client to manage cosmetic equipment actors and more.
 */
UCLASS(BlueprintType, Blueprintable, NotPlaceable)
class ITEMIZATIONCORERUNTIME_API AEquippableInventory : public AInventoryBase
{
	GENERATED_BODY()

public:
	AEquippableInventory(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~ End UObject Interface

protected:
	/** Replicated list of equippable inventory item entries. */
	UPROPERTY(ReplicatedUsing = OnRep_EquippableInventory, BlueprintReadOnly, Category = Inventory)
	FInventoryItemContainer EquippableInventory;

	UFUNCTION()
	virtual void OnRep_EquippableInventory();
};
