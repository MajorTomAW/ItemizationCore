// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "ItemComponentData.h"
#include "Pickup/PickupCreationData.h"

#include "ItemComponentData_PickupBase.generated.h"

/** Base item data for providing pickup-able logic to an item.
 * It is highly recommended that you create your own subclass of this to feed in more data about the pickup.
 */
USTRUCT(DisplayName="Pickup Item Data Base")
struct ITEMIZATIONCORE_API FItemComponentData_PickupBase : public FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData_PickupBase();

	/** Constructs a pickup creation data. */
	virtual FPickupCreationData GetPickupCreationData(const FInventoryItemEntry& ItemEntry, const FInventoryHandle& InventoryHandle) const;

public:
	/** Actor class to spawn that acts as a pickupable actor in the world. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Pickup, meta=(MustImplement="/Script/ItemizationCore.ItemPickupInterface"))
	TSubclassOf<AActor> PickupActorClass;
};
