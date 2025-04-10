// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ItemizationWorldSubsystem.generated.h"

class AInventoryBase;
/**
 * 
 */
UCLASS()
class ITEMIZATIONCORERUNTIME_API UItemizationWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

protected:
	/** List of all registered inventories in the world. */
	UPROPERTY()
	TArray<TWeakObjectPtr<AInventoryBase>> InventoryList;
};
