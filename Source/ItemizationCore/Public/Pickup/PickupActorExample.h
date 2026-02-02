// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "ItemPickupInterface.h"
#include "GameFramework/Actor.h"
#include "Items/InventoryItemEntry.h"
#include "PickupActorExample.generated.h"

UCLASS()
class APickupActorExample
	: public AActor
	, public IItemPickupInterface
{
	GENERATED_BODY()

public:
	APickupActorExample();

	//~ Begin IItemPickupInterface
	virtual void SetupPickupWithCreationData(const FPickupCreationData& CreationData) override;
	virtual FPickupPickedUpEvent* GetOnPickedUpDelegate() override { return &OnPickedUpEvent; }
	virtual void DespawnPickup() override;
	virtual const FInventoryItemEntry& GetPrimaryPickupItemEntry() const override;
	//~ End IItemPickupInterface

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = PickupActorExample, Replicated)
	FInventoryItemEntry PickupItem;

	UPROPERTY()
	FPickupPickedUpEvent OnPickedUpEvent;
};
