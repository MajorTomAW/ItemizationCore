// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinition.h"
#include "PlaysetItemDefinition.generated.h"

/**
 * Minimal version of an item definition that can be placed in the world.
 * Used to define presets and other structures.
 */
UCLASS()
class ITEMIZATIONCORERUNTIME_API UPlaysetItemDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UPlaysetItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Whether this playset should adjust for world collision when dragged into the world. */
	UPROPERTY(EditDefaultsOnly, Category = "Placement", meta = (DisplayName = "Adjust for World Collision"))
	uint32 bAdjustForWorldCollision : 1;

	/** The extent of the playset that will be used to adjust for world collision. */
	UPROPERTY(EditDefaultsOnly, Category = "Placement", meta = (DisplayName = "Collision Extent"))
	FVector CollisionExtent;

	/** The default location offset to use when spawning the playset. */
	UPROPERTY(EditDefaultsOnly, Category = "Placement|Offset", meta = (DisplayName = "Default Location"))
	FVector DefaultLocation;

	/** The default rotation offset to use when spawning the playset. */
	UPROPERTY(EditDefaultsOnly, Category = "Placement|Offset", meta = (DisplayName = "Default Rotation"))
	FRotator DefaultRotation;

	/** The actor class map used as a fast lookup for spawning actors. */
	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TMap<TSoftClassPtr<AActor>, int32> ActorClassCount;

	/** The list of actors that will be spawned when the item definition is placed in the world. */
	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TArray<FItemActorData> ActorDataList;

	/** The source actor blueprint that will be used to spawn the playset. If not set, the playset will try to spawn actors from the data list. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actor Data", AssetRegistrySearchable, meta = (ForceShowPluginContent = true))
	TSoftClassPtr<AActor> SourceActorBlueprint;
};
