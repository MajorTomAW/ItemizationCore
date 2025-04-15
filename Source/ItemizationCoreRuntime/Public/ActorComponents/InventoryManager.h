// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventorySlotHandle.h"
#include "Components/ActorComponent.h"
#include "Items/InventoryItemEntry.h"
#include "InventoryManager.generated.h"

class AInventory;
class UInventorySetupDataBase;
enum class EItemizationInventoryCreationType : uint8;
class AInventoryBase;

/** Actor component used to manage inventories. */
UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent), ClassGroup=Inventory, EditInlineNew,
	HideCategories=(Object,LOD,Lighting,Transform,Sockets,TextureStreaming,VirtualTexture))
class ITEMIZATIONCORERUNTIME_API UInventoryManager : public UActorComponent
{
	GENERATED_BODY()
	friend class AInventoryBase;

public:
	UInventoryManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Returns the root inventory that this component is managing. */
	UFUNCTION(BlueprintCallable, Category=Inventory)
	AInventory* GetRootInventory() const;

	/** ---------------------------------------------------------------------------------------------------------------
	 * Inventory Management
	 *
	 * These functions are used to manage the inventory system.
	 * 1. Adding Items
	 *		– GiveItem() Only the server can give items.
	 *			Will try to fill up existing item stacks in the inventory before creating new instances.
	 *			If we couldn't fill up enough existing stacks, it will create a new item instance and replicate it down to clients.
	 *			This also works recursively for the number of items that couldn't be added to the inventory.
	 *
	 *			E.g. If we have 12 items to add, and the max stack size is 5,
	 *			it will create two stacks of 5x items and one stack of 2x items.
	 *
	 *			Result:
	 *			– 5x items	(5/5)
	 *			– 5x items	(5/5)
	 *			– 2x items	(2/5)
	 *
	 *			The created item instance has a cached FInventoryItemEntryHandle that can be used to reference the item.
	 *		
	 * 2. Removing Items
	 *		– RemoveItem() Only the server can remove items.
	 -----------------------------------------------------------------------------------------------------------------*/

	/**
	 * Adds an item to the inventory.
	 * This will be ignored if the actor is not authoritative, as items can only be added on the server.
	 * @param	ItemEntry	The constructed FInventoryItemEntry to add. Will be evaluated by the item components before being added.
	 * @param	OutExcess [OUT]	Any excess items that couldn't be added to the inventory.
	 * @returns	The unique item handle of the item that was added to the inventory, or an invalid one if the item couldn't be added.
	 */
	FInventoryItemHandle GiveItem(const FInventoryItemEntry& ItemEntry, int32& OutExcess) const;

	/** Overloads of the GiveItem(*) function. */
	FInventoryItemHandle GiveItem(const FInventoryItemEntry& ItemEntry) const;

	/**
	 * Adds an item to the inventory.
	 * This will be ignored if the actor is not authoritative, as items can only be added on the server.
	 * @param	ItemDefinition	The item definition that should be added to the inventory.
	 * @param	StackCount		The number of items to add to the inventory.
	 * @param	OutExcess [OUT]	Any excess items that couldn't be added to the inventory.
	 * @returns	The unique item handle of the item that was added to the inventory, or an invalid one if the item couldn't be added.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory, meta=(DisplayName="Give Item", Scriptname=GiveItem))
	UPARAM(DisplayName=Handle) FInventoryItemHandle K2_GiveItem(UItemDefinition* ItemDefinition, int32 StackCount, int32& OutExcess);
	  

public:
	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	virtual void InitializeComponent() override;
	virtual void PostNetReceive() override;
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	//~ End UObject Interface

	/** Will look for an inventory manager somewhere in the owner chain and grab its inventory. */
	void LookForInventoryManager();

	/** Spawns a new inventory system based on the setup data. */
	virtual void SpawnInventory(UWorld* World, AActor* Owner, APlayerController* PlayerController = nullptr);

protected:
	/** The inventory that this component is managing. */
	UPROPERTY(Transient)
	TWeakObjectPtr<AInventory> RootInventory;

	/** The unique identifier of this inventory manager. */
	UPROPERTY()
	FGuid InventoryId;

	/** The creation policy for the inventory. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Inventory)
	EItemizationInventoryCreationType CreationPolicy;

	/** The setup data to use for the creation of the inventory. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Inventory, meta=(EditCondition="CreationPolicy == EItemizationInventoryCreationType::SetupData"))
	TSoftObjectPtr<UInventorySetupDataBase> InventorySetupData;
};
