// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UItemDefinition;
struct FSoftObjectPath;

namespace UE::ItemizationCore
{
	namespace Display
	{
		/** Tries to find the icon of the specified item definition. */
		ITEMIZATIONCORERUNTIME_API FSoftObjectPath FindItemIcon(const UItemDefinition* InItem);

		/** Gets any description of the specified item definition. Preferably the long one. */
		ITEMIZATIONCORERUNTIME_API FText GatherItemDesc_Any(const UItemDefinition* InItem);

		/** Gets the display name of the specified item definition. */
		ITEMIZATIONCORERUNTIME_API FText GatherItemDisplayName(const UItemDefinition* InItem);
	}
}
