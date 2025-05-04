// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "InventoryConfig.h"
#include "InventoryHandle.h"
#include "Components/GameFrameworkComponent.h"
#include "Interfaces/InventoryOwnerInterface.h"
#include "Items/InventoryItemSlot.h"
#include "InventoryComponent.generated.h"


/** Base class for all inventory managers. */
UCLASS(Config=Game, ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent), Abstract)
class ITEMIZATIONCORERUNTIME_API UInventoryComponent 
	: public UGameFrameworkComponent
	, public IGameplayTagAssetInterface
	, public IInventoryOwnerInterface
{
	GENERATED_BODY()

public:
	UInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin IInventoryOwnerInterface
	virtual AInventoryBase* GetInventory() const override;
	//~ End IInventoryOwnerInterface

	//~ Begin IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~ End IGameplayTagAssetInterface

	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitProperties() override;
	virtual void InitializeComponent() override;
	virtual void PostNetReceive() override;
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

protected:
	/** Creates the actual inventory actor storing it in the handle. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	virtual void CreateInventory();

	/** Called right after the inventory was spawned or set by replication. */
	virtual void OnInventoryCreated(AInventoryBase* Inventory);

	virtual void InitInventoryGroups();

protected:
	/** The inventory class to use for this inventory manager. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = InventoryConfig)
	TSubclassOf<AInventoryBase> InventoryClass;
	
	/** Whether this component should automatically acquire an inventory object on begin-play. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = InventoryConfig)
	uint8 bShouldAcquireInventoryOnInitialize:1;
	
	/** Config data for this inventory manager. This should only be set through the editor as we currently don't support runtime changes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = InventoryConfig)
	TObjectPtr<UInventoryConfig> InventoryConfig;

	/** Handle that points to the inventory this manager is associated with. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_InventoryHandle, Category = Inventory)
	FInventoryHandle InventoryHandle;

	//@TODO: This is only temporary to make sure the inventory doesn't GC. But I need to make this better later
	UPROPERTY()
	TObjectPtr<AInventoryBase> AuthorityInventory;

	UFUNCTION()
	virtual void OnRep_InventoryHandle();

	UPROPERTY()
	TMap<FGameplayTag, FInventoryItemSlotGroup> ItemSlotGroups;
};
