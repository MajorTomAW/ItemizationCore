// Author: Tom Werner (dc: majort), 2026

#pragma once

#include "UObject/Interface.h"

#include "IInventoryOwnerInterface.generated.h"

#define UE_API ITEMIZATIONCORE_API

class AInventoryBase;

/** Interface which can be used to find an inventory on an arbitrary actor. */
UINTERFACE(BlueprintType, MinimalAPI)
class UInventoryOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class IInventoryOwnerInterface
{
	GENERATED_BODY()

public:
	/** Returns the owning inventory this interface is associated with. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category=Inventory)
	UE_API AInventoryBase* GetInventory() const;
	virtual AInventoryBase* GetInventory_Implementation() const = 0;
};

#undef UE_API
