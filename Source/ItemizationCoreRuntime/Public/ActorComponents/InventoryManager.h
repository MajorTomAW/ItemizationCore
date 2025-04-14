// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManager.generated.h"

class UInventorySetupDataBase;
enum class EItemizationInventoryCreationType : uint8;
class AInventoryBase;

/** Actor component used to manage inventories. */
UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent), ClassGroup=Inventory)
class ITEMIZATIONCORERUNTIME_API UInventoryManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Returns the root inventory that this component is managing. */
	UFUNCTION(BlueprintCallable, Category=Inventory)
	AInventoryBase* GetRootInventory() const;

public:
	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	virtual void InitializeComponent() override;
	virtual void PostNetReceive() override;
	virtual void OnRegister() override;
	//~ End UObject Interface

protected:
	/** The inventory that this component is managing. */
	UPROPERTY()
	TWeakObjectPtr<AInventoryBase> RootInventory;

	/** The unique identifier of this inventory manager. */
	UPROPERTY()
	FGuid InventoryId;

	/** The creation policy for the inventory. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Inventory)
	EItemizationInventoryCreationType CreationPolicy;

	/** The setup data to use for the creation of the inventory. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Inventory, meta=(EditCondition="CreationPolicy == EItemizationInventoryCreationType::SetupData"))
	TSoftObjectPtr<UInventorySetupDataBase> InventorySetupData;

	/** The default root inventory class to spawn if no setup data is provided. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Inventory, meta=(EditCondition="CreationPolicy == EItemizationInventoryCreationType::Runtime"))
	TSoftClassPtr<AInventoryBase> RootInventoryClass;
};
