// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryEquipmentEntry.h"
#include "Components/ActorComponent.h"
#include "EquipmentManager.generated.h"

/**
 * Manages the equipment of an actor.
 */
UCLASS(ClassGroup = (Itemization), meta = (BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class ITEMIZATIONCORERUNTIME_API UEquipmentManager : public UActorComponent
{
	GENERATED_UCLASS_BODY()

	/** Static getter to find the equipment manager on an actor. */
	static UEquipmentManager* GetEquipmentManager(AActor* Actor);

	//~ Begin UObject Interface
	virtual void PreNetReceive() override;
	virtual void BeginPlay() override;
	//~ End UObject Interface

	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const override;
	//~ End UActorComponent Interface

protected:
	/** The replicated equipment list. */
	UPROPERTY(Replicated, BlueprintReadOnly, Transient, Category = "Equipment")
	FInventoryEquipmentContainer EquipmentList;
};
