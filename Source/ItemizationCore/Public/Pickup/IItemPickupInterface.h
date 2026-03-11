// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemList.h"

#include "UObject/Interface.h"

#include "IItemPickupInterface.generated.h"

class UItemDefinitionBase;
struct FInventoryItemEntry;
class IInventoryOwnerInterface;
struct FPickupCreationData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPickupPickedUpEvent, TScriptInterface<IItemPickupInterface>, SelfActor, APawn*, InteractingPawn);

/**
 * Pickupable interface used for actors that can have an item associated for picking up into an inventory.
 * It's highly recommended to create your own custom pickup actor that implements this interface.
 */
UINTERFACE(MinimalAPI, BlueprintType, meta=(CannotImplementInterfaceInBlueprint))
class UItemPickupInterface : public UInterface
{
	GENERATED_BODY()
};

class IItemPickupInterface
{
	GENERATED_BODY()

public:
	/** Initializes the pickup actor with the given creation data. */
	virtual void SetupPickupWithCreationData(const FPickupCreationData& CreationData) = 0;

	/** Called when this pickup has been given to a target inventory. */
	virtual void OnPickupGivenTo(TScriptInterface<IInventoryOwnerInterface> InventoryOwner) {};

	/** Returns the delegate to the picked-up event. */
	virtual FPickupPickedUpEvent* GetOnPickedUpDelegate() = 0;

	/** Checks whether the pickup can by picked up by the associated pawn. */
	UFUNCTION(BlueprintCallable, Category=Pickup)
	virtual bool CanBePickedUpBy(const APawn* InteractingPawn) const { return true; };

	/** Makes this pickup despawn. */
	UFUNCTION(BlueprintCallable, Category=Pickup)
	virtual void DespawnPickup() = 0;

	/** Returns the primary item entry stored in this pickupable actor.
	 * Note that the FInventoryItemEntry is just a copy of the original item that was dropped.
	 * Therefore, by modifying this item entry, you only modify the one inside the pickup and not inside the inventory.
	 */
	UFUNCTION(BlueprintCallable, Category=Pickup)
	virtual const FInventoryItemEntry&	GetPrimaryPickupItemEntry() const = 0;

	/** Returns the primary item definition stored in this pickupable actor. */
	UFUNCTION(BlueprintCallable, Category=Pickup)
	virtual const UItemDefinitionBase* GetPrimaryPickupItemDefinition() const { return GetPrimaryPickupItemEntry().GetItemDefinition(); }
};
