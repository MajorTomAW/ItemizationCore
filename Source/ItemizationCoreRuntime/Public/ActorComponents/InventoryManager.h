// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManager.generated.h"

class UInventorySetupDataBase;
enum class EItemizationInventoryCreationType : uint8;
class AInventoryBase;

/** Actor component used to manage inventories. */
UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent), MinimalAPI, ClassGroup=Inventory)
class UInventoryManager : public UActorComponent
{
	GENERATED_BODY()

public:
	ITEMIZATIONCORERUNTIME_API UInventoryManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** The inventory that this component is managing. */
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TWeakObjectPtr<AInventoryBase> RootInventory;

	/** The creation policy for the inventory. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Inventory)
	EItemizationInventoryCreationType CreationPolicy;

	/** The globally unique id of the created inventory. */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = Inventory)
	FGuid InventoryId;

	/** The setup data to use for the creation of the inventory. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Inventory)
	TSoftObjectPtr<UInventorySetupDataBase> InventorySetupData;
};
