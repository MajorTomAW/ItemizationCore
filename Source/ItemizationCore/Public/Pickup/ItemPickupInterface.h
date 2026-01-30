// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "ItemPickupInterface.generated.h"

struct FPickupCreationData;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FPickupPickedUpEvent, TScriptInterface<IItemPickupInterface>, SelfActor, APawn*, InteractingPawn);

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
};
