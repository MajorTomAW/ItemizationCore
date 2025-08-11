// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "InventoryHandle.h"
#include "Components/ActorComponent.h"
#include "Components/GameFrameworkComponent.h"
#include "Inventory/IInventoryOwnerInterface.h"
#include "Inventory/InventorySlotGroup.h"
#include "InventoryComponent.generated.h"

#define MY_API ITEMIZATIONCORE_API

class IInventoryItemInstanceInterface;
class UItemDefinitionBase;
class UInventoryItemInstance;
/** Base class for all inventory managers. */
UCLASS(Config=Game, ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent), MinimalAPI, HideCategories=(ComponentTick, Activation, Tags))
class UInventoryComponent
	: public UGameFrameworkComponent
	, public IGameplayTagAssetInterface
	, public IInventoryOwnerInterface
{
	GENERATED_BODY()

public:
	MY_API UInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin IGameplayTagAssetInterface Implementation
	MY_API virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~ End IGameplayTagAssetInterface Implementation

	//~ Begin IInventoryOwnerInterface Implementation
	MY_API virtual AInventoryBase* GetInventory() const override;
	//~ End IInventoryOwnerInterface Implementation

	//~ Begin UObject Interface
	MY_API virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	MY_API virtual void PostInitProperties() override;
	MY_API virtual void InitializeComponent() override;
	MY_API virtual void PostNetReceive() override;
	MY_API virtual void OnRegister() override;
	MY_API virtual void BeginPlay() override;

#if WITH_EDITOR
	MY_API virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

public:
	UFUNCTION(BlueprintCallable, Category=Inventory)
	TScriptInterface<IInventoryItemInstanceInterface> FindItemInstanceByHandle(const FInventoryItemHandle& ItemHandle) const;
	
	UFUNCTION(BlueprintCallable, Category=Inventory)
	TArray<TScriptInterface<IInventoryItemInstanceInterface>> GetInventoryItems() const;

	UFUNCTION(BlueprintCallable, Category=Inventory, meta=(Categories="Inventory.Group"))
	TArray<TScriptInterface<IInventoryItemInstanceInterface>> GetInventoryItemsInGroup(FGameplayTag Group) const;

	UFUNCTION(BlueprintCallable, Category=Inventory, meta=(Categories="Inventory.Group"))
	TArray<TScriptInterface<IInventoryItemInstanceInterface>> GetInventoryItemsInGroups(TArray<FGameplayTag> Groups) const;

	/**
	 * Attempts to give an item to the inventory.
	 * Will be ignored if the actor is not auhoritative.
	 * 
	 * @param ItemDefinition	The item definition to give.
	 * @param StackCount		The number of items to give.
	 * @param SourceObject		Optional source object giving the item.
	 * @param OutNumCouldNotAdd [OUT] The amount of items that could not be added to the inventory due to stack size limits or other restrictions.
	 * @returns The handle to the item that was added, or an invalid handle if the item could not be added.
	 */
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintAuthorityOnly)
	FInventoryItemHandle TryGiveItem(UItemDefinitionBase* ItemDefinition, int32 StackCount, UObject* SourceObject, int32& OutNumCouldNotAdd);

protected:
	/** Creates the actual inventory actor storing it in the handle. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	MY_API virtual void CreateInventory();

	/** Called right after the inventory was spawned or set by replication. */
	MY_API virtual void OnInventoryCreated(AInventoryBase* Inventory);

	MY_API virtual void InitInventoryGroups();

public:
	/** The inventory class to use for this inventory manager. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = InventoryConfig)
	TSubclassOf<AInventoryBase> InventoryClass;
	
	/** Whether this component should automatically acquire an inventory object on begin-play. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = InventoryConfig)
	uint8 bShouldAcquireInventoryOnInitialize:1;

	/** If true, the inventory will be attached to the owner of this component. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = InventoryConfig)
	uint8 bAttachInventoryToOwner:1;
	
	/** Config data for this inventory manager. This should only be set through the editor as we currently don't support runtime changes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = InventoryConfig)
	TObjectPtr<class UInventoryConfigAsset> InventoryConfig;

	/** Handle that points to the inventory this manager is associated with. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_InventoryHandle, Category = Inventory)
	FInventoryHandle InventoryHandle;

	//@TODO: This is only temporary to make sure the inventory doesn't GC. But I need to make this better later
	UPROPERTY()
	TObjectPtr<AInventoryBase> AuthorityInventory;

	UFUNCTION()
	MY_API virtual void OnRep_InventoryHandle();

	UPROPERTY()
	TMap<FGameplayTag, FInventoryItemSlotGroup> ItemSlotGroups;

	UPROPERTY(Replicated)
	FInventorySlotContainer ItemSlotContainer;
};

#undef MY_API