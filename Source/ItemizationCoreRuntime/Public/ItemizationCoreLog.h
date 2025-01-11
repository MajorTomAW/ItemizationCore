// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EngineDefines.h"

ITEMIZATIONCORERUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogInventorySystem, Display, All);

namespace Itemization
{
	ITEMIZATIONCORERUNTIME_API FString GetClientServerContextString(const UObject* Object);
	
#define ITEMIZATION_LOG(Verbosity, Format, ...) \
UE_LOG(LogInventorySystem, Verbosity, Format, ##__VA_ARGS__);

#define ITEMIZATION_C_LOG(Verbosity, Format, ...) \
UE_LOG(LogInventorySystem, Verbosity, TEXT("%s"), *FString::Printf(TEXT(Format), ##__VA_ARGS__));

#define ITEMIZATION_Net_LOG(Verbosity, Object, Format, ...) \
	UE_LOG(LogInventorySystem, Verbosity, TEXT("%s: ") Format, *GetClientServerContextString(Object), ##__VA_ARGS__)
}

