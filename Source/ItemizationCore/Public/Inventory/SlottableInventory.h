// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "InventoryBase.h"
#include "SlottableInventory.generated.h"

UCLASS()
class ITEMIZATIONCORE_API ASlottableInventory : public AInventoryBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASlottableInventory();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
