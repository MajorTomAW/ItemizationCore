// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ItemComponentDataList.generated.h"

struct FInstancedStruct;

/** 
 * This struct is used to hold a list of item component data.
 * It is used for serialization and deserialization of item component data.
 * It is also used for replication of item component data.
 */
USTRUCT()
struct FItemComponentDataList
{
	GENERATED_BODY()

public:
	TArray<FInstancedStruct*> DataList;
};