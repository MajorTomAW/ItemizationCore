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
#include "Items/ItemDefinitionBase.h"
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
	/** Returns the item instance associated to the given item id. */
	UFUNCTION(BlueprintCallable, Category=Inventory)
	TScriptInterface<IInventoryItemInstanceInterface> FindItemInstanceById(const FInventoryItemId& ItemId) const;

	/** Returns a list of all inventory item instances. */
	UFUNCTION(BlueprintCallable, Category=Inventory)
	TArray<TScriptInterface<IInventoryItemInstanceInterface>> GetInventoryItems() const;

	/**
	 * Attempts to give an item to the inventory.
	 * Will be ignored if the actor is not authoritative.
	 *
	 * @param ItemDefinition	The item definition to give.
	 * @param StackCount		The number of items to give.
	 * @param SourceObject		Optional source object giving the item.
	 * @param GroupTag			Tag of the group to add the item to.
	 * @param OutNumCouldNotAdd [OUT] The amount of items that could not be added to the inventory due to stack size limits or other restrictions.
	 * @returns The handle to the item that was added, or an invalid handle if the item could not be added.
	 */
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintAuthorityOnly, meta=(Categories="Inventory.Group"))
	FInventoryItemId TryGiveItem(UItemDefinitionBase* ItemDefinition, int32 StackCount, UObject* SourceObject, FGameplayTag GroupTag, int32& OutNumCouldNotAdd);

	/**
	 * Attempts to remove an item from the inventory.
	 * Will be ignored if the actor is not authoritative.
	 * 
	 * @param ItemDefinition	The item definition to remove. 
	 * @param NumRemove			The number of items to remove.
	 * @param GroupTag			Tag of the group to remove the item from.
	 * @return The number of items that got removed.
	 */
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintAuthorityOnly, meta=(Categories="Inventory.Group"))
	int32 TryRemoveItemByDefinition(const UItemDefinitionBase* ItemDefinition, int32 NumRemove, FGameplayTag GroupTag);

	/**
	 * Attempts to remove an item from the inventory.
	 * Will be ignored if the actor is not authoritative.
	 * 
	 * @param ItemId			The unique item id of the item to be removed. 
	 * @param NumRemove			The number of items to remove.
	 * @param GroupTag			Tag of the group to remove the item from.
	 * @return The number of items that got removed.
	 */
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintAuthorityOnly, meta=(Categories="Inventory.Group"))
	int32 TryRemoveItemById(const FInventoryItemId& ItemId, int32 NumRemove, FGameplayTag GroupTag);

	/**
	 * Attempts to remove an item from the inventory.
	 * Will be ignored if the actor is not authoritative.
	 * 
	 * @param ItemInstance		The instance to be removed. 
	 * @param NumRemove			The number of items to remove.
	 * @param GroupTag			Tag of the group to remove the item from.
	 * @return The number of items that got removed.
	 */
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintAuthorityOnly, meta=(Categories="Inventory.Group"))
	int32 TryRemoveItem(TScriptInterface<IInventoryItemInstanceInterface> ItemInstance, int32 NumRemove, FGameplayTag GroupTag);

protected:
	/** Creates the actual inventory actor storing it in the handle. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	MY_API virtual void CreateInventory();

	/** Called right after the inventory was spawned or set by replication. */
	MY_API virtual void OnInventoryCreated(AInventoryBase* Inventory);

	MY_API virtual void InitInventoryGroups(AInventoryBase* Inventory);

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
};

#undef MY_API
