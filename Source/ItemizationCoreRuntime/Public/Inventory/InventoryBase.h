// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/InventoryItemEntry.h"
#include "InventoryBase.generated.h"

struct FInventoryTransaction_GiveItem;
/** Inventory class that manages an inventory list. */
UCLASS(HideCategories=(Input,Movement,Collision,Rendering,Physics), BlueprintType, Blueprintable)
class ITEMIZATIONCORERUNTIME_API AInventoryBase : public AActor
{
	GENERATED_BODY()

public:
	AInventoryBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	//~ End UObject Interface

	//~ Begin AActor Interface
	virtual void PostInitializeComponents() override;
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

	/** Adds an item to the inventory. */
	virtual FInventoryItemHandle GiveItem(const FInventoryItemEntry& ItemEntry, int32& OutExcess, FInventoryTransaction_GiveItem& Transaction);
	virtual void OnRemoveItem(FInventoryItemEntry& ItemEntry);
	virtual void OnGiveItem(FInventoryItemEntry& ItemEntry);
	
protected:
	/** Evaluates the given ItemEntry and checks if it can be added to the inventory. */
	virtual void EvaluateItemEntry(const FInventoryItemEntry& ItemEntry, FInventoryTransaction_GiveItem& InOutTransaction);

	/** Internal version of GiveItem. Don't call this directly. */
	virtual FInventoryItemHandle NativeGiveItem(const FInventoryItemEntry& ItemEntry, FInventoryTransaction_GiveItem& Transaction, int32& OutExcess);

	virtual bool CanMergeItems(const FInventoryItemEntry& ThisEntry, const FInventoryItemEntry& OtherEntry) const;
	virtual void MergeItems(const FInventoryItemEntry& ThisEntry, FInventoryItemEntry& OtherEntry, int32& OutExcess) const;
	virtual bool CanCreateNewStack(const FInventoryItemEntry& ItemEntry, const FInventoryTransaction_GiveItem& Transaction);
	virtual bool ShouldCreateNewInstanceOfItem(const FInventoryItemEntry& ItemEntry) const;
	virtual UInventoryItemInstance* CreateNewInstanceOfItem(FInventoryItemEntry& ItemEntry);

	virtual void NotifyItemAdded(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);
	virtual void NotifyItemRemoved(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);
	virtual void NotifyItemChanged(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);

protected:
	/** Replicated list of inventory item entries. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing=OnRep_InventoryList, Category=Inventory)
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
