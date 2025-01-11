// Copyright Epic Games, Inc. All Rights Reserved.


#include "Components/EquipmentComponentData_Default.h"

#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"


void FEquipmentComponentData_Default::QuerySpawnParams(
	TArray<FItemizationEquipmentSpawnQuery>& Queries, const UObject* ContextObject) const
{
	const USceneComponent* AttachTarget = Cast<USceneComponent>(ContextObject);

	if (const APawn* Pawn = Cast<APawn>(ContextObject))
	{
		AttachTarget = Pawn->GetRootComponent();
	}

	if (const ACharacter* Character = Cast<ACharacter>(ContextObject))
	{
		AttachTarget = Character->GetMesh();
	}
	
	for (const auto& ActorToSpawn : EquipmentActors)
	{
		FItemizationEquipmentSpawnQuery& Query = Queries.Add_GetRef(FItemizationEquipmentSpawnQuery());
		Query.SetActorClass(ActorToSpawn.ActorToSpawn);
		Query.SetSocketName(ActorToSpawn.SocketName);
		Query.SetRelativeTransform(ActorToSpawn.RelativeTransform);
		Query.SetAttachTarget(const_cast<USceneComponent*>(AttachTarget));
	}
}
