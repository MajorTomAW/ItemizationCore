// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryBase.h"
#include "Slots/InventorySlotEntry.h"

#include "SlottableInventory.generated.h"

UCLASS(BlueprintType, Blueprintable, NotPlaceable)
class ASlottableInventory : public AInventoryBase
{
	GENERATED_BODY()

public:
	ASlottableInventory(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UObject Interface

protected:
	/** Replicated inventory slot list. */
	UPROPERTY(ReplicatedUsing = OnRep_InventorySlots)
	FInventorySlotContainer InventorySlots;

	UFUNCTION()
	virtual void OnRep_InventorySlots();
};
