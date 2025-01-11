// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/EquipmentComponentData.h"

#include "EquipmentComponentData_Default.generated.h"

class AActor;
class APawn;
class UObject;

/**
 * Data-struct that holds information about a piece of equipment that will be spawned.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FEquipmentComponentActorToSpawn
{
	GENERATED_BODY()

public:
	/** The actor to spawn. */
	UPROPERTY(EditAnywhere, Category = "Equipment")
	TSoftClassPtr<AActor> ActorToSpawn;

	/** The attachment socket name. */
	UPROPERTY(EditAnywhere, Category = "Equipment", meta = (AnimNotifyBoneName = true))
	FName SocketName = NAME_None;

	/** The transform of the actor to spawn. */
	UPROPERTY(EditAnywhere, Category = "Equipment")
	FTransform RelativeTransform = FTransform::Identity;
};

/**
 * Default equipment component data structure used to spawn a piece equipment.
 */
USTRUCT(DisplayName = "Default Equipment")
struct ITEMIZATIONCORERUNTIME_API FEquipmentComponentData_Default : public FEquipmentComponentData
{
	GENERATED_BODY()

public:
	/** The equipment actor to spawn. */
	UPROPERTY(EditAnywhere, Category = "Equipment")
	TArray<FEquipmentComponentActorToSpawn> EquipmentActors;

protected:
	//~ Begin FEquipmentComponentData Interface
	virtual void QuerySpawnParams(TArray<FItemizationEquipmentSpawnQuery>& Queries, const UObject* ContextObject) const override;
	//~ End FEquipmentComponentData Interface
};
