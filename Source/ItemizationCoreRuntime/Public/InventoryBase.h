// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Items/InventoryItemEntry.h"
#include "InventoryBase.generated.h"

struct FInventoryStartingItem;
class UInventorySetupDataBase;
enum class EItemizationInventoryType : uint8;
class UInventoryManager;
class UObject;
class AInfo;
class UActorChannel;
class FOutBunch;
struct FFrame;

/**
 * Inventory base class that is created for every inventory system in the game.
 * An AInventorBase is mostly associated with an UInventoryManager that is linked to it.
 * Instead of having the UInventoryManager be the base class, we have this AInfo class to enable replication for
 * world actors that aren't replicated but still need to own an inventory system.
 */
UCLASS(BlueprintType, Blueprintable, NotPlaceable, MinimalAPI)
class AInventoryBase : public AInfo
{
	GENERATED_BODY()

public:
	AInventoryBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~ End UObject Interface

	/** Initializes the inventory with the given setup data. */
	virtual void GrantStartingItems(TArray<const FInventoryStartingItem*> StartingItems);

public:
	/** Describes the type of inventory this is. */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = Inventory)
	EItemizationInventoryType InventoryType;

protected:
	/** Replicated list of inventory item entries. */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryList, BlueprintReadOnly, Category = Inventory)
	FInventoryItemContainer InventoryList;

	UFUNCTION()
	virtual void OnRep_InventoryList();

protected:
	/** Potential pointer to a parent inventory that owns this inventory. */
	UPROPERTY()
	TObjectPtr<AInventoryBase> ParentInventory;

	/** List of inventories that are children of this inventory. */
	UPROPERTY()
	TArray<TObjectPtr<AInventoryBase>> ChildInventoryList;
};
