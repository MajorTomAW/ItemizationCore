// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"

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
UCLASS(NotPlaceable, MinimalAPI, Abstract)
class AInventoryBase : public AInfo
{
	GENERATED_BODY()

public:
	AInventoryBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void Init(const UInventorySetupDataBase* InSetupData, AInventoryBase* InParent, const TArray<AInventoryBase*> InChildren = {});

	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~ End UObject Interface
	
protected:
	/** Describes the type of inventory this is. */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = Inventory)
	EItemizationInventoryType InventoryType;

public:
	/** Potential pointer to a parent inventory that owns this inventory. */
	UPROPERTY(BlueprintReadOnly, Category = Inventory, Replicated)
	TObjectPtr<AInventoryBase> ParentInventory;

	/** List of inventories that are children of this inventory. */
	UPROPERTY(BlueprintReadOnly, Category = Inventory, Replicated)
	TArray<TObjectPtr<AInventoryBase>> ChildInventoryList;
};
