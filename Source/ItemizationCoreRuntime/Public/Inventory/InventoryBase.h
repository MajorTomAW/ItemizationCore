// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enums/EItemizationInventoryType.h"
#include "GameFramework/Info.h"

#include "InventoryBase.generated.h"

class AInventory;
struct FInventoryPropertiesBase;
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
	virtual void Init(AInventoryBase* InParent, const TArray<AInventoryBase*> InChildren = {});
	virtual void PostInitInventory();

	virtual void SetProperties(const FInventoryPropertiesBase* InProperties, bool bIsPlayerInventory)
	{
		InventoryType = bIsPlayerInventory ? EItemizationInventoryType::Player : EItemizationInventoryType::World;
		InventoryProperties = InProperties;
	}

	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void PreNetReceive() override;
	//~ End UObject Interface

	template <typename PropertyType = FInventoryPropertiesBase>
	const PropertyType* GetInventoryProperties() const
	{
		static_assert(TIsDerivedFrom<PropertyType, FInventoryPropertiesBase>::IsDerived,
			"PropertyType must be derived from FInventoryPropertiesBase");
		return InventoryProperties ? static_cast<const PropertyType*>(InventoryProperties) : nullptr;
	}
	const FInventoryPropertiesBase* GetInventoryProperties() const { return InventoryProperties; }
	
protected:
	/** Describes the type of inventory this is. */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = Inventory)
	EItemizationInventoryType InventoryType;

	/**
	 * Number that defines the overall limit of this inventory.
	 * By default, this is set to -1, which means no limit.
	 * Items won't count towards this limit if they don't have the "CountTowardsInventoryLimit" Trait set.
	 *
	 * Helps in managing and restricting the number of items a player can carry.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Inventory, meta=(ClampMin=-1, UIMin=-1))
	int32 InventoryLimit = -1;
	
	/** Cached off pointer to the inventory properties. */
	mutable const FInventoryPropertiesBase* InventoryProperties;

public:
	/** Potential pointer to a parent inventory that owns this inventory. */
	UPROPERTY(BlueprintReadOnly, Category = Inventory, Replicated)
	TObjectPtr<AInventoryBase> ParentInventory;

	/** List of inventories that are children of this inventory. */
	UPROPERTY(BlueprintReadOnly, Category = Inventory, Replicated)
	TArray<TObjectPtr<AInventoryBase>> ChildInventoryList;

	/**
	 * Transient unique id associated with the owner of this inventory.
	 * Will be used to detect previous ownership in case the player disconnects.
	 */
	UPROPERTY()
	FUniqueNetIdRepl OwnerUniqueId;
};
