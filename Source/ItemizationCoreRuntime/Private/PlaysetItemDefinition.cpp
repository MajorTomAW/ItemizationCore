// Copyright © 2024 Playton. All Rights Reserved.


#include "PlaysetItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PlaysetItemDefinition)

UPlaysetItemDefinition::UPlaysetItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bAdjustForWorldCollision(false)
	, CollisionExtent(ForceInit)
	, DefaultLocation(ForceInit)
	, DefaultRotation(ForceInit)
{
	PrimaryAssetType = TEXT("PlaysetItemDefinition");
}
