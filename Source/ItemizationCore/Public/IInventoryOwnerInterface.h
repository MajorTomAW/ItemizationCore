// Author: Tom Werner (dc: majort), 2026

#pragma once

#include "UObject/Interface.h"

#include "IInventoryOwnerInterface.generated.h"

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
	AInventoryBase* GetInventory() const;
	virtual AInventoryBase* GetInventory_Implementation() const = 0;
};
