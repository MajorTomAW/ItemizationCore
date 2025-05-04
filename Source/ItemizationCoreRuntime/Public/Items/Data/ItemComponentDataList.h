// Author: Tom Werner (MajorT), 2025

#pragma once

#include "StructUtils/InstancedStruct.h"

#include "ItemComponentDataList.generated.h"

struct FInstancedStruct;

USTRUCT()
struct FItemComponentDataList
{
	GENERATED_BODY()

public:
	/** Instanced list of item component data structs. */
	UPROPERTY()
	TArray<FInstancedStruct> DataList;
};