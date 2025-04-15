// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryBase.h"
#include "Items/InventoryItemEntry.h"

#include "Inventory.generated.h"


struct FInventoryChangeMessage;
struct FInventoryTransaction_GiveItem;

/**
 * The inventory class that holds a list of items.
 * Each item that lives in this class is purely a data object and doesn't have any physical representation.
 */
UCLASS(BlueprintType, Blueprintable, NotPlaceable)
class ITEMIZATIONCORERUNTIME_API AInventory : public AInventoryBase
{
	GENERATED_BODY()
	friend struct FInventoryItemEntry;
	friend struct FInventoryItemContainer;

public:
	AInventory(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Initializes the inventory with the given setup data. */
	virtual void GrantStartingItems(TArray<const FInventoryStartingItem*> StartingItems);

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
	 * Gives an item to the inventory.
	 * This will be ignored if the actor is not authoritative, as items can only be added on the server.
	 * Furthermore, this function shouldn't be called directly, but rather through the inventory manager component.
	 * @param ItemEntry The constructed FInventoryItemEntry to add. Will be evaluated by the item components before being added.
	 * @param Transaction	The transaction that stores payload data about the GiveItem() action.
	 * @param OutExcess		[OUT]	Any excess items that couldn't be added to the inventory.
	 * @returns The unique item handle of the item that was added to the inventory, or an invalid one if the item couldn't be added.
	 */
	virtual FInventoryItemHandle GiveItem(const FInventoryItemEntry& ItemEntry, const FInventoryTransaction_GiveItem& Transaction, int32& OutExcess);

protected:
	/**
	 * Evaluates the given ItemEntry and checks if it can be added to the inventory.
	 * Will fill in any important data by iterating over all item components.
	 * @param ItemEntry 
	 * @param InOutTransaction 
	 */
	virtual void EvaluateItemEntry(const FInventoryItemEntry& ItemEntry, FInventoryTransaction_GiveItem& InOutTransaction);

	virtual FInventoryItemHandle NativeGiveItem(const FInventoryItemEntry& ItemEntry, const FInventoryTransaction_GiveItem& Transaction, int32& OutExcess);

	/** Will be called from RemoveItem or from OnRep. */
	virtual void OnRemoveItem(FInventoryItemEntry& ItemEntry);

	/** Will be called from GiveItem or from OnRep. Initializes the given item instance. */
	virtual void OnGiveItem(FInventoryItemEntry& ItemEntry);

public:
	/** Inventory Event delegate. */
	DECLARE_EVENT_OneParam(AInventory, FInventoryItemEvent, const FInventoryChangeMessage& /*Payload*/)

	/** Delegate that gets called whenever a new item was added to the inventory. */
	FInventoryItemEvent OnItemAddedDelegate;

	/** Delegate that gets called right before an item is removed from the inventory. */
	FInventoryItemEvent OnItemRemovedDelegate;

	/** Delegate that gets called whenever an item was changed. */
	FInventoryItemEvent OnItemChangedDelegate;
	
protected:
	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	//~ End UObject Interface

protected:
	/** Replicated list of inventory item entries. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = OnRep_InventoryList, Category = Inventory)
	FInventoryItemContainer InventoryList;

	/** OnRep function that gets called whenever the InventoryList is replicated. */
	UFUNCTION()
	virtual void OnRep_InventoryList();
	FTimerHandle OnRep_InventoryListTimerHandle;

	/** Full list of all item instances that were added via an FInventoryItemEntry. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInventoryItemInstance>> AllItemInstances;

	/** Returns the full list of all item instances. */
	const TArray<UInventoryItemInstance*>& GetAllItemInstances() const { return AllItemInstances; }

	/** Adds a new item instance to the replicated sub object list. */
	void AddReplicatedItemInstance(UInventoryItemInstance* ItemInstance);

	/** Removes an item instance from the replicated sub object list. */
	void RemoveReplicatedItemInstance(UInventoryItemInstance* ItemInstance);

	/**
	 * Called to mark an item entry dirty for replication.
	 * bWasAddOrChange is an important flag to determine whether the entire array needs to be replicated,
	 * or if we can just replicate the item delta entry.
	 * 
	 * @param ItemEntry		The item entry to mark dirty.
	 * @param bWasAddOrChange	True, if the item was added or changed. False, if the item was removed.
	 */
	void MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange = false);

private:
	/** Returns the mutable full list of all item instances. */
	TArray<TObjectPtr<UInventoryItemInstance>>& GetAllItemInstances_Mutable() { return AllItemInstances; }
};
