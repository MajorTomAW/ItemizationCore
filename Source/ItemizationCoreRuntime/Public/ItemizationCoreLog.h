// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EngineDefines.h"

ITEMIZATIONCORERUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogInventorySystem, Display, All);

#if PLATFORM_DESKTOP
#define ITEMIZATION_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogInventorySystem, Verbosity, Format, ##__VA_ARGS__); \
}
#endif