// Author: Tom Werner (dc: majort), 2026

#pragma once

#include "CoreMinimal.h"
#include "IInventoryOwnerInterface.h"
#include "Components/GameFrameworkComponent.h"

#include "InventoryComponent.generated.h"

#define UE_API ITEMIZATIONCORE_API

class AInventoryBase;
/** Component responsible for spawning an inventory. Usually placed on a player controller. */
UCLASS(Config=Game, ClassGroup=(Inventory), MinimalAPI, meta=(BlueprintSpawnableComponent))
class UInventoryComponent
	: public UGameFrameworkComponent
	, public IInventoryOwnerInterface
{
	GENERATED_BODY()

public:
	UE_API UInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryInitialized, AInventoryBase* /*Inventory*/)

	//~ Begin IInventoryOwnerInterface Implementation
	UE_API virtual AInventoryBase* GetInventory_Implementation() const override;
	//~ End IInventoryOwnerInterface Implementation

	//~ Begin UObject Interface
	UE_API virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void PostInitProperties() override;
	UE_API virtual void InitializeComponent() override;
	UE_API virtual void PostNetReceive() override;
	UE_API virtual void OnRegister() override;
	UE_API virtual void BeginPlay() override;

#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface


	/** Will register with inventory initialized delegate, or called immediately if already initialized. */
	UE_API void CallOrRegister_OnInventoryInitialized(FOnInventoryInitialized::FDelegate&& Delegate);

protected:
	/** Creates the actual inventory actor storing it in the handle. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UE_API virtual void CreateInventory();

	/** Called right after the inventory was spawned or set by replication. */
	UE_API virtual void OnInventoryCreated(AInventoryBase* MyInventory);

	/** Override this, if you want to setup your inventory before its readiness gets broadcasted. */
	UE_API virtual void SetupInventory(AInventoryBase* MyInventory);

	/** Initializes the inventory slots and groups. */
	UE_API virtual void InitInventoryGroups(AInventoryBase* MyInventory);

	UFUNCTION()
	void OnRep_Inventory();

public:
	/** The inventory class to spawn with this component. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category=Inventory)
	TSubclassOf<AInventoryBase> InventoryClass;

	/** Whether this component should automatically acquire an inventory object on begin-play. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	uint8 bShouldAcquireInventoryOnInitialize:1;

	/** If true, the inventory will be attached to the owner of this component. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	uint8 bAttachInventoryToOwner:1;

	/** Config data for this inventory manager. This should only be set through the editor as we currently don't support runtime changes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	TObjectPtr<class UInventoryConfig> InventoryConfig;

	/** The replicated inventory that was created. */
	UPROPERTY(ReplicatedUsing=OnRep_Inventory)
	TObjectPtr<AInventoryBase> Inventory;

private:
	/** Delegate to be called when the inventory gets initialized. */
	FOnInventoryInitialized OnInventoryInitialized;
};

#undef UE_API
