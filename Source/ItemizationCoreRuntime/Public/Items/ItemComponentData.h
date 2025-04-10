// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ItemComponentData.generated.h"

/** 
 * This struct represents the base item component data.
 * It can be used to define custom logic for items.
 * These are stored as instanced structs in the inventory system and have logic that can be called during runtime.
 */
USTRUCT(BlueprintType)
struct FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData();
};