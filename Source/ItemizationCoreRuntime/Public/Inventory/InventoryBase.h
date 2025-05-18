// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "InventoryHandle.h"
#include "GameFramework/Actor.h"
#include "Items/InventoryItemEntry.h"
#include "Transactions/InventoryItemMoveOp.h"
#include "Transactions/InventoryOpCache.h"
#include "InventoryBase.generated.h"

struct FInventoryItemMoveOp;
struct FInventoryChangeMessage;
struct FInventoryTransaction_GiveRemoveItem;

/** Inventory item event delegate. */
DECLARE_MULTICAST_DELEGATE_OneParam(FInventoryItemEvent, const FInventoryChangeMessage&)

#define MY_API ITEMIZATIONCORERUNTIME_API

/** Inventory class that manages an inventory list. */
UCLASS(HideCategories=(Input,Movement,Collision,Rendering,Physics), BlueprintType, Blueprintable, MinimalAPI)
class AInventoryBase : public AActor
{
	GENERATED_BODY()

public:
	MY_API AInventoryBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~ Begin UObject Interface
	MY_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	MY_API virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	MY_API virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	//~ End UObject Interface

	//~ Begin AActor Interface
	MY_API virtual void PostInitializeComponents() override;
	//~ End AActor Interface

public:
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
	 *			For example, if we have 12 items to add, and the max stack size is 5,
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

	MY_API virtual TInventoryOpHandle<FInventoryItemMoveOp> MoveItem(FInventoryItemMoveOp::Params&& Params);

	/** Adds an item to the inventory. */
	MY_API virtual FInventoryItemHandle GiveItem(const FInventoryItemEntry& ItemEntry, int32& OutExcess, FInventoryTransaction_GiveRemoveItem& Transaction);

	/** Removes an item from the inventory. */
	MY_API virtual bool RemoveItem(const FInventoryItemHandle& ItemHandle, FInventoryTransaction_GiveRemoveItem& Transaction, int32& OutMissing);
	
	MY_API virtual void OnRemoveItem(FInventoryItemEntry& ItemEntry);
	MY_API virtual void OnGiveItem(FInventoryItemEntry& ItemEntry);

	/** Delegate that gets called whenever a new item was added to the inventory. */
	FInventoryItemEvent OnItemAddedDelegate;

	/** Delegate that gets called right before an item is removed from the inventory. */
	FInventoryItemEvent OnItemRemovedDelegate;

	/** Delegate that gets called whenever an item was changed. */
	FInventoryItemEvent OnItemChangedDelegate;

	/** Handle for outside inventory access. Gets set by the inventory component. */
	UPROPERTY()
	FInventoryHandle InventoryHandle;
	
protected:
	/** Evaluates the given ItemEntry and checks if it can be added to the inventory. */
	MY_API virtual void EvaluateItemEntry(const FInventoryItemEntry& ItemEntry, FInventoryTransaction_GiveRemoveItem& InOutTransaction);

	/** Internal version of GiveItem. Don't call this directly. */
	MY_API virtual FInventoryItemHandle NativeGiveItem(const FInventoryItemEntry& ItemEntry, FInventoryTransaction_GiveRemoveItem& Transaction, int32& OutExcess);

	/** Internal version of RemoveItem. Don't call this directly. */
	MY_API bool NativeRemoveItem(const auto& Operator, FInventoryTransaction_GiveRemoveItem& Transaction, int32& OutMissing, bool bRecursive = true);

	/** Checks whether two item entries can be merged in a single stack. */
	MY_API virtual bool CanMergeItems(const FInventoryItemEntry& ThisEntry, const FInventoryItemEntry& OtherEntry) const;

	/** Actually performs the merge action by merging 'ThisEntry' into 'OtherEntry'. */
	MY_API virtual void MergeItems(const FInventoryItemEntry& ThisEntry, FInventoryItemEntry& OtherEntry, int32& OutExcess) const;

	/** Checks whether an item can create an entire new stack in this inventory. Might be restricted by traits. */
	MY_API virtual bool CanCreateNewStack(const FInventoryItemEntry& ItemEntry, const FInventoryTransaction_GiveRemoveItem& Transaction);

	/** Checks whether the item wants a new item instance. Some items don't require having one. */ 
	MY_API virtual bool ShouldCreateNewInstanceOfItem(const FInventoryItemEntry& ItemEntry) const;

	/** Actually creates the new instance and adds it to the tracking list. */
	MY_API virtual UInventoryItemInstance* CreateNewInstanceOfItem(FInventoryItemEntry& ItemEntry);

	MY_API virtual void NotifyItemAdded(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);
	MY_API virtual void NotifyItemRemoved(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);
	MY_API virtual void NotifyItemChanged(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);

protected:
	/** Replicated list of inventory item entries. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing=OnRep_InventoryList, Category=Inventory)
	FInventoryItemContainer InventoryList;

	/** OnRep function that gets called whenever the InventoryList is replicated. */
	UFUNCTION()
	MY_API virtual void OnRep_InventoryList();
	FTimerHandle OnRep_InventoryListTimerHandle;

	/** Full list of all item instances that were added via an FInventoryItemEntry. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInventoryItemInstance>> AllItemInstances;

	/** Returns the full list of all item instances. */
	MY_API const TArray<UInventoryItemInstance*>& GetAllItemInstances() const { return AllItemInstances; }

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
	MY_API void MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange = false);
	
private:
	/** Returns the mutable full list of all item instances. */
	TArray<TObjectPtr<UInventoryItemInstance>>& GetAllItemInstances_Mutable() { return AllItemInstances; }

	/** Cached inventory operations. */
	FInventoryOpCache OpCache;
};


#undef MY_API