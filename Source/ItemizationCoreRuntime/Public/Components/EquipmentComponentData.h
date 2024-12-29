// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "EquipmentComponentData.generated.h"

/**
 * A single query struct used to spawn equipment.
 */
struct ITEMIZATIONCORERUNTIME_API FItemizationEquipmentSpawnQuery
{
	friend class UInventoryEquipmentInstance;
	FItemizationEquipmentSpawnQuery()
		: SocketName(NAME_None)
		, RelativeTransform(FTransform::Identity)
	{
	}

public:
	bool IsValid() const
	{
		return !ActorToSpawn.IsNull();
	}
	
	void SetSocketName(const FName& InSocketName)
	{
		SocketName = InSocketName;
	}

	void SetAttachTarget(USceneComponent* InAttachTarget)
	{
		if (InAttachTarget == nullptr)
		{
			return;
		}
		
		AttachTarget = InAttachTarget;
	}

	void SetActorClass(const UClass* ActorClass)
	{
		check(ActorClass->IsChildOf(AActor::StaticClass()));
		ActorToSpawn = ActorClass;
	}

	void SetActorClass(const TSoftClassPtr<AActor>& ActorClass)
	{
		if (ActorClass.IsNull())
		{
			return;
		}

		ActorToSpawn = ActorClass;
	}

	void SetRelativeTransform(const FTransform& InRelativeTransform)
	{
		RelativeTransform = InRelativeTransform;
	}

	void SetRelativeLocation(const FVector& InRelativeLocation)
	{
		RelativeTransform.SetLocation(InRelativeLocation);
	}

	void SetRelativeRotation(const FRotator& InRelativeRotation)
	{
		RelativeTransform.SetRotation(InRelativeRotation.Quaternion());
	}

	void SetRelativeScale(const FVector& InRelativeScale)
	{
		RelativeTransform.SetScale3D(InRelativeScale);
	}
	
protected:
	/** The name of the socket to attach the equipment to. */
	FName SocketName;

	/** The target component to attach the equipment to. */
	TWeakObjectPtr<USceneComponent> AttachTarget;

	/** The equipment actor to spawn. */
	TSoftClassPtr<AActor> ActorToSpawn;

	/** The relative transform of the equipment actor. */
	FTransform RelativeTransform;
};

/**
 * Equipment component data structure used to spawn equipment. 
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FEquipmentComponentData
{
	GENERATED_BODY()

public:
	virtual ~FEquipmentComponentData() = default;

	/** Called to evaluate the spawn query params for the piece of equipment. */
	virtual void QuerySpawnParams(TArray<FItemizationEquipmentSpawnQuery>& Queries, const UObject* ContextObject) const {}
};
