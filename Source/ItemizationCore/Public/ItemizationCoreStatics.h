// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "InventorySlotId.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemizationCoreStatics.generated.h"

/** Library with a lot of useful helper functions */
UCLASS()
class UItemizationCoreStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Creates a new item slot id. */
	UFUNCTION(BlueprintPure, Category=Inventory)
	static FInventorySlotId MakeInventorySlotId(int64 Row, int64 Column);
};
