// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryBase.h"
#include "Items/InventoryItemEntry.h"

#include "Inventory.generated.h"


struct FInventoryDescriptorData;
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
	 *			The created item instance has a cached FInventoryItemHandle that can be used to reference the item.
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

	/**
	 * Returns the item entry for the given item handle.
	 * This will iterate over all item entries and return the first one that matches the given handle.
	 */
	virtual FInventoryItemEntry* FindItemEntryFromHandle(const FInventoryItemHandle& ItemHandle) const;

protected:
	/**
	 * Evaluates the given ItemEntry and checks if it can be added to the inventory.
	 * Will fill in any important data by iterating over all item components.
	 * @param ItemEntry 
	 * @param InOutTransaction 
	 */
	virtual void EvaluateItemEntry(const FInventoryItemEntry& ItemEntry, FInventoryTransaction_GiveItem& InOutTransaction);

	/**
	 * Internal version of GiveItem().
	 * Don't call this directly as it doesn't perform any validation and evaluation.
	 */
	virtual FInventoryItemHandle NativeGiveItem(const FInventoryItemEntry& ItemEntry, const FInventoryTransaction_GiveItem& Transaction, int32& OutExcess);

	/**
	 * Checks whether two item entries can be merged into a single stack.
	 * In this case, 'ThisEntry' will try to merge into 'OtherEntry'.
	 * This will also check for potential restrictions and other factors.
	 * @param	ThisEntry	The item entry that we will try to merge into the other entry.
	 * @param	OtherEntry	The item entry that we will try to merge into.
	 * @returns True, if the items can be merged into a single stack.
	 */
	virtual bool CanMergeItems(const FInventoryItemEntry& ThisEntry, const FInventoryItemEntry& OtherEntry) const;

	/**
	 * Performs the actual merge of two item entries.
	 * Don't call this manually, but rather use the CanMergeItems() function to check if the items can be merged.
	 * @param	OutExcess	[OUT] The number of excess items that couldn't be merged into the other entry.
	 */
	virtual void MergeItems(const FInventoryItemEntry& ThisEntry, FInventoryItemEntry& OtherEntry, int32& OutExcess) const;

	/**
	 * Checks whether we can create a new stack for a given item entry.
	 * This will check for potential restrictions and other factors.
	 * @param ItemEntry	The entry to check if we can create a new stack for.
	 * @param Transaction	The transaction that stores payload data about the GiveItem() action.
	 * @returns True, if we can create a new stack for the given item entry.
	 */
	virtual bool CanCreateNewStack(const FInventoryItemEntry& ItemEntry, const FInventoryTransaction_GiveItem& Transaction);

	/**
	 * Checks if we need to create a new item instance for the given item entry.
	 * Will always return true by default, but can be overridden by subclasses.
	 */
	virtual bool ShouldCreateNewInstanceOfItem(const FInventoryItemEntry& ItemEntry) const;

	/**
	 * Creates a new item instance for the given item entry.
	 * Stores it in the item entry and adds it to the replicated sub object list.
	 */
	virtual UInventoryItemInstance* CreateNewInstanceOfItem(FInventoryItemEntry& ItemEntry);
	
	/** Will be called from RemoveItem or from OnRep. */
	virtual void OnRemoveItem(FInventoryItemEntry& ItemEntry);

	/** Will be called from GiveItem or from OnRep. Initializes the given item instance. */
	virtual void OnGiveItem(FInventoryItemEntry& ItemEntry);

	/** Called to broadcast any item delegates and events. */
	virtual void NotifyItemAdded(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);
	virtual void NotifyItemRemoved(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);
	virtual void NotifyItemChanged(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);

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

	//~ Begin AActor Interface
	virtual void PostInitializeComponents() override;
	//~ End AActor Interface

protected:
	/** Replicated list of inventory item entries. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = OnRep_InventoryList, Category = Inventory)
	FInventoryItemContainer InventoryList;

	/** OnRep function that gets called whenever the InventoryList is replicated. */
	UFUNCTION()
	virtual void OnRep_InventoryList();
	FTimerHandle OnRep_InventoryListTimerHandle;

	/** The actor that owns this component logically. */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryOwner, Transient)
	TObjectPtr<AActor> InventoryOwner;

	/** The actor that is the physical representation of the owner. */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryOwner, Transient)
	TObjectPtr<AActor> InventoryAvatar;

	/** OnRep for the owner actor. */
	UFUNCTION()
	virtual void OnRep_InventoryOwner();

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

protected:
	/**
	 * Initializes the inventory descriptor with the given actor and avatar.
	 * Doesn't need to be called manually unless you want to control both the avatar and owner of the inventory.
	 */
	virtual void InitInventoryDescriptor(AActor* InOwner, AActor* InAvatar);

	/** Clears out and invalidates the inventory descriptor. */
	virtual void ClearInventoryDescriptor();

	/** Cached data about the inventory system. */
	TSharedPtr<FInventoryDescriptorData> InventoryDescriptor;

public:
	/** Sets the owner actor. */
	void SetInventoryOwner(AActor* NewOwnerActor);
	AActor* GetInventoryOwner() const { return InventoryOwner.Get(); }

	/** Used to set the avatar actor directly, bypassing the automatic setting. */
	void SetInventoryAvatar_Direct(AActor* NewAvatarActor);
	AActor* GetInventoryAvatar_Direct() const { return InventoryAvatar.Get(); }

	/** Used to set the avatar actor. */
	void SetInventoryAvatar(AActor* NewAvatarActor);
	AActor* GetInventoryAvatar() const;

private:
	/** Returns the mutable full list of all item instances. */
	TArray<TObjectPtr<UInventoryItemInstance>>& GetAllItemInstances_Mutable() { return AllItemInstances; }
};
