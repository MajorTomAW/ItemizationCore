// Author: Tom Werner (dc: majort), 2026

#pragma once

#include "CoreMinimal.h"
#include "IItemPickupInterface.h"
#include "GameFramework/Actor.h"
#include "ExamplePickupActor.generated.h"

UCLASS()
class ITEMIZATIONCORE_API AExamplePickupActor
	: public AActor
	, public IItemPickupInterface
{
	GENERATED_BODY()

public:
	AExamplePickupActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void SetupPickupWithCreationData(const FPickupCreationData& CreationData) override;
	virtual void DespawnPickup() override;
	virtual const FInventoryItemEntry& GetPrimaryPickupItemEntry() const override;
	virtual FPickupPickedUpEvent* GetOnPickedUpDelegate() override { return nullptr; }

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated)
	FInventoryItemEntry ItemEntry;

	FPickupPickedUpEvent OnPickedUpDelegate;
};
