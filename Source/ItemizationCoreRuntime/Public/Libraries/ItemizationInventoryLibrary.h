// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemHandle.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemizationInventoryLibrary.generated.h"

struct FInventoryTrackableOp;
class UItemDefinitionBase;
class IInventoryOwnerInterface;
/**
 * 
 */
UCLASS()
class ITEMIZATIONCORERUNTIME_API UItemizationInventoryLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static FInventoryItemHandle GiveItem(TScriptInterface<IInventoryOwnerInterface> InventoryOwner, UItemDefinitionBase* ItemDefinition, int32 StackCount, int32& ExcessAmount);

	/** Performs a transaction on the inventory. */
	UFUNCTION(BlueprintCallable, Category=Itemization)
	static void PerformTransaction(
		TScriptInterface<IInventoryOwnerInterface> InventoryOwner,
		FInventoryTrackableOp& Transaction);
};
