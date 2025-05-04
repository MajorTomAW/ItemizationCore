// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "InventoryOwnerInterface.generated.h"

class AInventoryBase;
class UInventoryComponent;

/** Interface which can be used to pass around an inventory reference without needing to specify the type. */
UINTERFACE(BlueprintType, meta=(CannotImplementInterfaceInBlueprint), MinimalAPI)
class UInventoryOwnerInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IInventoryOwnerInterface
{
	GENERATED_IINTERFACE_BODY()
	
public:
	/** Returns the owning inventory this interface is associated with. */
	UFUNCTION(BlueprintCallable, Category="Inventory System")
	virtual AInventoryBase* GetInventory() const = 0;
};
