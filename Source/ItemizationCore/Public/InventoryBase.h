// Author: Tom Werner (dc: majort), 2026

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemId.h"
#include "InventoryItemList.h"
#include "InventorySlotList.h"
#include "GameFramework/Actor.h"
#include "Items/ItemDefinitionBase.h"
#include "Pickup/PickupCreationData.h"
#include "InventoryBase.generated.h"

#define UE_API ITEMIZATIONCORE_API


class UInventoryConfig;
struct FInventoryItemSlot;

UCLASS(MinimalAPI)
class AInventoryBase : public AActor
{
	GENERATED_BODY()

public:
	AInventoryBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	static UE_API const int32 REMOVE_ENTIRE_STACK;

	/** Initializes this inventory's slots using the inventory config asset. */
	UE_API virtual void InitializeInventorySlots(const UInventoryConfig* InConfig);

	/**
	 * Gives an item to the inventory.
	 * Will try to place it into the next unoccupied item slot if the item can't be placed into overflow.
	 * Overflow meaning that the item is inside the inventory but not in the inventory "grid".
	 */
	UE_API UItemInstanceBase* GiveItem(const UItemDefinitionBase* ItemDefinition, int32 NumToGive, UObject* SourceObject, int32& OutNumCouldNotGive);

	/** Performs a server rpc to call GiveItem() on this inventory. */
	UFUNCTION(Server, Reliable)
	UE_API void Server_GiveItem(const UItemDefinitionBase* ItemDefinition, int32 NumToGive, UObject* SourceObject);

	/**
	 * Removes an item from the inventory.
	 *
	 * @param Item The item instance to look for.
	 * @param NumToRemove Negative values means the entire stack.
	 * @returns The number of items that were removed.
	 */
	UE_API int32 RemoveItem(const UItemInstanceBase* Item, int32 NumToRemove = REMOVE_ENTIRE_STACK);
	UE_API int32 RemoveItemById(const FInventoryItemId& ItemId, int32 NumToRemove = REMOVE_ENTIRE_STACK);

	/** Removes all items from the inventory. */
	UE_API virtual void RemoveAllItems();

	/** Performs a server rpc to call RemoveItem() on this inventory. */
	UFUNCTION(Server, Reliable)
	UE_API void Server_RemoveItem(const UItemInstanceBase* Item, int32 NumToRemove = REMOVE_ENTIRE_STACK);

	/**
	 * Attempts to place an item into an inventory slot.
	 * This is an advanced function, consider calling the wrappers on UInventoryComponent instead.
	 *
	 * @returns The item instance created during the process. Will be nullptr when failed.
	 */
	UE_API UItemInstanceBase* PlaceItemInSlot(const UItemDefinitionBase* ItemDefinition, int32 NumToGive, UObject* SourceObject, const FInventorySlotId& SlotId, const FGameplayTag& GroupTag, int32& OutNumCouldNotGive);

	/** Performs a server rpc to call PlaceItemInSlot() on this inventory. */
	UFUNCTION(Server, Reliable)
	UE_API void Server_PlaceItemInSlot(const UItemDefinitionBase* ItemDefinition, int32 NumToGive, UObject* SourceObject, const FInventorySlotId& SlotId, const FGameplayTag& GroupTag);

	/**
	 * Swaps the contents of the specified slots A and B.
	 * Will only update the slot's assigned item id without altering the ItemEntry.
	 */
	UE_API void SwapItemSlots(const FInventorySlotId& SlotA, const FGameplayTag& GroupTagA, const FInventorySlotId& SlotB, const FGameplayTag& GroupTagB);

	/**
	 * Removes and drops an item to the ground.
	 *
	 * @param Item The item instance to drop
	 * @param NumToDrop Negative values means the entire stack
	 * @return The pickup actor, should inherit IItemPickupInterface
	 */
	UE_API virtual AActor* DropItem(const UItemInstanceBase* Item, int32 NumToDrop = REMOVE_ENTIRE_STACK);
	UE_API virtual AActor* DropItem(const FInventoryItemId& ItemId, int32 NumToDrop = REMOVE_ENTIRE_STACK);

	/** Removes and drops all items to the ground. */
	UE_API virtual TArray<AActor*> DropAllItems();

	/** Creates a new item entry and initializes it with default values. */
	UE_API FInventoryItemEntry CreateNewItemEntry(const UItemDefinitionBase* InItemDefinition, int32 InCount, UObject* InSourceObject) const;


	/** Returns an item entry associated by its id. */
	UE_API FInventoryItemEntry* FindItemEntryById(const FInventoryItemId& ItemId) const;

	/** Returns the first item entry with matching item definition. */
	UE_API FInventoryItemEntry* FindFirstItemEntryByDefinition(const UItemDefinitionBase* ItemDefinition) const;

	/** Returns an item instanced associated by its item id. */
	UE_API UItemInstanceBase* GetItem(const FInventoryItemId& ItemId) const;

	/** Returns an item slot associated by its slot id. */
	UE_API FInventoryItemSlot* FindItemSlotBySlotId(const FInventorySlotId& SlotId, const FGameplayTag& GroupTag) const;

	/** Returns an item slot associated by its occupying item id. */
	UE_API FInventoryItemSlot* FindItemSlotByItemId(const FInventoryItemId& ItemId, const FGameplayTag& GroupTag) const;

	/** Returns the inventory group the specified item is in. */
	UE_API FGameplayTag GetInventoryGroupTag(const FInventoryItemEntry& ItemEntry) const;
	UE_API FGameplayTag GetInventoryGroupTag(const FInventoryItemId& ItemId) const;

	/** Returns a list of all group tags. */
	UE_API TArray<FGameplayTag> GetAllItemSlotGroupTags() const;

	/** Returns a list of all item slots in the specified group. */
	UE_API TArray<const FInventoryItemSlot*> GetItemSlotsInGroup(const FGameplayTag& GroupTag) const;

	/** Returns the next unoccupied item slot for the specified item, checking restrictions etc. */
	UE_API FInventoryItemSlot* GetNextAvailableItemSlot(const FInventoryItemEntry& ItemEntry, const FGameplayTag& GroupTag) const;

	/** Returns the full inventory list. */
	const FInventoryItemList& GetInventoryList() const { return InventoryList; }

	/** Returns the full slot list. */
	const FInventorySlotList& GetSlotList() const { return SlotList; }

	/** Returns the number of rows in the given inventory group. */
	UE_API int32 GetNumRowsInGroup(FGameplayTag GroupTag) const;

	/** Returns the number of columns in the given inventory group. */
	UE_API int32 GetNumColumnsInGroup(FGameplayTag GroupTag) const;

	/** Returns an item instance in the given slot with group. */
	UE_API UItemInstanceBase* GetItemInSlot(const FInventorySlotId& SlotId, const FGameplayTag& GroupTag) const;

	/** Returns true if this inventory contains the specified item. */
	UE_API bool ContainsItem(const UItemInstanceBase* ItemInstance) const;
	UE_API bool ContainsItem(const FInventoryItemId& ItemId) const;

	/** Creates the pickup creation data to use for spawning a pickup actor. */
	UE_API FPickupCreationData MakePickupCreationData(const FInventoryItemEntry& ItemEntry) const;

	/** Spawns a pickup actor using the given pickup creation data. */
	UE_API AActor* SpawnPickupActor(const FPickupCreationData& PickupCreationData) const;

	/** Called right after a pickup was spawned. */
	UE_API virtual void PostSpawnPickupActor(const FPickupCreationData& PickupCreationData, AActor* SpawnedPickup) const {}


	/** Checks whether we can create a new item stack for the given item entry. */
	UE_API virtual bool CanCreateNewStack(const FInventoryItemEntry& ItemEntry) const;

	/** Checks whether an item can be combined with an existing stack to fill larger stacks first. */
	UE_API virtual bool CanAutoCombineStacks(const UItemDefinitionBase* ItemDefinition) const;

	/** Caches whether two items can be combined into a single stack. */
	UE_API virtual bool CanCombineItems(const FInventoryItemEntry& This, const FInventoryItemEntry& Other) const;

	/** Checks whether an item can be removed from the inventory. */
	UE_API virtual bool CanRemoveItem(const FInventoryItemEntry& ItemEntry) const;

	/** Checks whether the given item entry can be placed inside the specified item slot. */
	UE_API virtual bool CanPlaceItemInSlot(const FInventoryItemEntry& ItemEntry, const FInventoryItemSlot& ItemSlot) const;



	/** Called when an item was just given to the inventory. Used to initialize it. */
	UE_API virtual void OnGiveItem(FInventoryItemEntry& ItemEntry);

	/** Called when an item is about to be removed from the inventory. */
	UE_API virtual void OnRemoveItem(FInventoryItemEntry& ItemEntry);

	/** Called when an item slot was just added to this inventory. */
	UE_API virtual void OnAddItemSlot(FInventoryItemSlot& InventorySlot);

	/** Called when an item slot is about to be removed from this inventory. */
	UE_API virtual void OnRemoveItemSlot(FInventoryItemSlot& InventorySlot);

	/** Called when the contents of an item slot have changed. Either from authority or replication. */
	UE_API virtual void OnItemSlotChanged(FInventoryItemSlot& InventorySlot, const FInventoryItemId& LastItemId, const FInventoryItemId& NewItemId);

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FGenericItemChangeEvent, const FInventoryItemEntry& /*Item*/, const int32& /*LastCount*/, const int32& /*NewCount*/);

	/** Notify that gets called whenever an item has changed, called by the inventory itself or due to replication. */
	UE_API virtual void NotifyItemChanged(const FInventoryItemEntry& ItemThatChanged, const int32& LastStackSize, const int32& NewStackSize) const;
	FGenericItemChangeEvent OnItemChangedDelegate;

	/** Notify that gets called whenever an item got added to the inventory, called by the inventory itself or due to replication. */
	UE_API virtual void NotifyItemAdded(const FInventoryItemEntry& ItemThatWasAdded, const int32& LastStackSize, const int32& NewStackSize) const;
	FGenericItemChangeEvent OnItemAddedDelegate;

	/** Notify that gets called whenever an item got removed to the inventory, called by the inventory itself or due to replication. */
	UE_API virtual void NotifyItemRemoved(const FInventoryItemEntry& ItemThatWasRemoved, const int32& LastStackSize, const int32& NewStackSize) const;
	FGenericItemChangeEvent OnItemRemovedDelegate;

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FGenericSlotChangedEvent, const FInventoryItemSlot& /*Slot*/, const FInventoryItemId& /*LastItemInSlot*/, const FInventoryItemId& /*NewItemInSlot*/)

	/** Notify that gets called by OnItemSlotChanged (also PostReplicatedChange) whenever an item slot has changed (aka an item was placed or removed from it). */
	UE_API virtual void NotifyItemSlotChanged(const FInventoryItemSlot& SlotThatChanged, const FInventoryItemId& LastItemInSlot, const FInventoryItemId& NewItemInSlot) const;
	FGenericSlotChangedEvent OnItemSlotChangedDelegate;

	/** Notify that gets called by OnAddItemSlot (also PostReplicatedAdd) whenever an item slot got added to the inventory. */
	UE_API virtual void NotifyItemSlotAdded(const FInventoryItemSlot& SlotThatWasAdded, const FInventoryItemId& LastItemInSlot, const FInventoryItemId& NewItemInSlot) const;
	FGenericSlotChangedEvent OnItemSlotAddedDelegate;

	/** Notify that gets called by OnRemoveItemSlot (also PostReplicatedRemove) whenever an item slot got removed from the inventory. */
	UE_API virtual void NotifyItemSlotRemoved(const FInventoryItemSlot& SlotThatWasRemoved, const FInventoryItemId& LastItemInSlot, const FInventoryItemId& NewItemInSlot) const;
	FGenericSlotChangedEvent OnItemSlotRemovedDelegate;

	/** Creates a new item instance, storing it inside  the item entry. */
	UE_API UItemInstanceBase* CreateNewItemInstance(FInventoryItemEntry& ItemEntry);

	/**
	 * Marks an item entry dirty for replication.
	 * bWasAddOrChange is an important flag to determine whether the entire array needs to be replicated,
	 * or if we can just replicate the item delta entry.
	 */
	UE_API void MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange = false, bool bForceMarkItemDirty = false);

	/**
	 * Marks a slot entry  for replication.
	 * bWasAddOrChange is an important flag to determine whether the entire array needs to be replicated,
	 * or if we can just replicate the item delta entry.
	 */
	UE_API void MarkItemSlotDirty(FInventoryItemSlot& ItemSlot, bool bWasAddOrChange = false, bool bForceMarkDirty = false);

	/** Called from FScopedInventoryLock. */
	UE_API void IncrementInventoryLock();
	UE_API void DecrementInventoryLock();
	bool IsInventoryLocked() const { return InventoryLockCount > 0; }
	void AddPendingItemAdd(const FInventoryItemEntry& ItemEntry);
	void AddPendingItemRemove(const FInventoryItemId& ItemId);

	//~ Begin AActor Interface
	UE_API virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	UE_API virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void PreNetReceive() override;
	UE_API virtual void PostNetReceive() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void PostInitializeComponents() override;
	//~ End AActor Interface

protected:
	/** Combines two items. */
	UE_API virtual void CombineItems(FInventoryItemEntry& This, FInventoryItemEntry& Other, int32& OutCouldNotCombine);

	/** Attempts to create a new item stack in this inventory. */
	UE_API virtual UItemInstanceBase* AttemptCreateNewStack(FInventoryItemEntry& ItemEntry, FInventoryItemId& OutItemId, const FGameplayTag& GroupTag, const int32& RemainingStacks, int32& OutCreatedStackSize);
	UE_API virtual UItemInstanceBase* AttemptCreateNewStack(FInventoryItemEntry& ItemEntry, FInventoryItemId& OutItemId, const FInventorySlotId& SlotId, const FGameplayTag& GroupTag, const int32& RemainingStacks, int32& OutCreatedStackSize);

	/** Adds an item instance to the replicated subobject list. */
	UE_API void AddReplicatedItemInstance(UItemInstanceBase* ItemInstance);

	/** OnRep function that gets called whenever the InventoryList is replicated. */
	UFUNCTION()
	UE_API virtual void OnRep_InventoryList();

protected:
	/** Replicated list of inventory item entries. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Transient, ReplicatedUsing=OnRep_InventoryList, Category=Inventory)
	FInventoryItemList InventoryList;

	/** Replicated list of inventory item slots. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Transient, Replicated, Category=Inventory)
	FInventorySlotList SlotList;

	/** The pickup actor class to use when spawning pickups. Must implement the IItemPickupInterface. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Pickup, meta=(MustImplement="/Script/ItemizationCore.ItemPickupInterface"))
	TSubclassOf<AActor> PickupActorClass;

	/** Replicated inventory config asset. */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated)
	TObjectPtr<const UInventoryConfig> InventoryConfigAsset;

	/** If greater 0, will lock the inventory list. */
	int32 InventoryLockCount;

	/** Items that will be removed when exiting the current inventory lock. */
	TArray<FInventoryItemId, TInlineAllocator<2>> ItemPendingRemoves;

	/** Items that will be added when exiting teh current inventory lock. */
	TArray<FInventoryItemEntry, TInlineAllocator<2>> ItemPendingAdds;

	struct FInventoryLockActiveChange
	{
		FInventoryLockActiveChange(AInventoryBase& InInventory,
			const TArray<FInventoryItemEntry, TInlineAllocator<2>>& InItemPendingAdds,
			const TArray<FInventoryItemId, TInlineAllocator<2>>& InItemPendingRemoves)
				: Inventory(InInventory)
				, Removes(InItemPendingRemoves)
				, Adds(InItemPendingAdds)
		{
			Inventory.InventoryLockActiveChanges.Add(this);
		}

		~FInventoryLockActiveChange()
		{
			Inventory.InventoryLockActiveChanges.Remove(this);
		}

		AInventoryBase& Inventory;
		TArray<FInventoryItemId, TInlineAllocator<2>> Removes;
		TArray<FInventoryItemEntry, TInlineAllocator<2>> Adds;
	};
	TArray<FInventoryLockActiveChange*> InventoryLockActiveChanges;
};
#undef UE_API
