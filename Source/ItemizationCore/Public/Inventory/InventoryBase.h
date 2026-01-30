// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "InventoryHandle.h"
#include "ItemizationCoreTypes.h"
#include "Items/InventoryItemEntry.h"
#include "Items/InventoryItemSlot.h"
#include "Items/ItemDefinitionBase.h"

#include "Operations/InventoryOp.h"
#include "Operations/InventoryOpCache.h"
#include "Operations/InventoryOp_GiveItem.h"
#include "Operations/InventoryOp_RemoveItem.h"
#include "Pickup/PickupCreationData.h"

#include "InventoryBase.generated.h"

struct FPickupCreationData;
struct FInventoryOp_PlaceItemInSlot;
class UInventoryConfigAsset;
class IInventoryItemInstanceInterface;
struct FInventoryOp_GiveItem;
class UInventoryItemInstance;
struct FInventoryItemEntry;

#define UE_API ITEMIZATIONCORE_API

/**
 * Inventory class that manages an inventory list.
 * Purely acts as a container for adding, removing and replicating an item array.
 * Additionally, items can be dropped on the floor using a pickup actor.
 * But besides that, this inventory doesn't have any other functionality but being an item container. 
 *
 * For more complex inventories see ASlottableInventory,
 * which supports things such as item groups and item slots.
 */
UCLASS(HideCategories=(Input,Movement,Collision,Rendering,Physics), BlueprintType, Blueprintable, MinimalAPI)
class AInventoryBase : public AActor
{
	GENERATED_BODY()
	friend struct FInventoryItemList;
	friend struct FInventoryItemEntry;

public:
	UE_API AInventoryBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Attempts to add an item to the inventory.
	 * This is an advanced function, consider calling the wrappers on UInventoryComponent instead.
	 * 
	 * @param Params Cached params about the give item action.
	 * @returns A pointer to the inventory operation. Can be used to get data such as how many items couldn't be added.
	 */
	UE_API virtual TInventoryOpPtr<FInventoryOp_GiveItem> GiveItem(FInventoryOp_GiveItem::FParams&& Params);

	/**
	 * Attempts to remove an item from the inventory.
	 * This is an advanced function, consider calling the wrappers on UInventoryComponent instead.
	 * 
	 * @param Params Cached params about the remove item action. Has item definition / id / instance / custom filter func,
	 * to find the item we want to remove.
	 * @return A pointer to the inventory operation. Can be used to get data such as how many items could/couldn't be removed.
	 */
	UE_API virtual TInventoryOpPtr<FInventoryOp_RemoveItem> RemoveItem(FInventoryOp_RemoveItem::FParams&& Params);

	/** Attempts to drop an item to the floor using the given item entry and count. */
	UE_API virtual AActor* DropItem(FInventoryItemEntry* ItemEntry, int32 NumToDrop = INDEX_NONE);
	UE_API virtual AActor* DropItem(TScriptInterface<IInventoryItemInstanceInterface> ItemInstance, int32 NumToDrop = INDEX_NONE);
	UE_API virtual AActor* DropItem(const FInventoryItemId& ItemId, int32 NumToDrop = INDEX_NONE);
	UE_API virtual AActor* DropItem(const UItemDefinitionBase* ItemDefinition, int32 NumToDrop = INDEX_NONE);

	/** Attempts to drop multiple items to the floor at once. */
	UE_API virtual AActor* DropItems(const TArray<FInventoryItemEntry*>& ItemEntries);
	UE_API virtual AActor* DropItems(const TArray<TScriptInterface<IInventoryItemInstanceInterface>>& ItemInstances);
	UE_API virtual AActor* DropItems(const TArray<const FInventoryItemEntry&>& ItemIds);
	UE_API virtual AActor* DropItems(const TArray<const UItemDefinitionBase*>& ItemDefinitions);

public:
	/** Checks whether an item can be combined with an existing stack to fill larger stacks first. */
	UE_API virtual bool CanAutoCombineStacks(const UItemDefinitionBase* ItemDefinition) const;

	/** Caches whether two items can be combined into a single stack. */
	UE_API virtual bool CanCombineItems(const FInventoryItemEntry& This, const FInventoryItemEntry& Other) const;

	/** Checks whether an item can be removed from the inventory. */
	UE_API virtual bool CanRemoveItem(const FInventoryItemEntry& ItemEntry) const;


	/** Checks whether the given ItemEntry should create a new item instance. This will always return true, unless overridden. */
	UE_API virtual bool ShouldCreateNewItemInstance(const FInventoryItemEntry& ItemEntry) const { return true; }

	/** Creates a new item instance for the given item entry. */
	TScriptInterface<IInventoryItemInstanceInterface> CreateNewItemInstance(FInventoryItemEntry& ItemEntry);

	/** Returns true if this inventory is limited. */
	UFUNCTION(BlueprintPure, Category=Inventory)
	bool IsInventoryLimited() const { return InventoryLimitType != EInventoryLimitType::None; }

	/** Returns an item instance associated by its id. */
	UFUNCTION(BlueprintPure, Category=Inventory)
	TScriptInterface<IInventoryItemInstanceInterface> FindItemInstanceById(const FInventoryItemId& ItemId) const;

	/** Returns all item instances. Including replicated and non-replicated ones. */
	UFUNCTION(BlueprintPure, Category=Inventory)
	TArray<TScriptInterface<IInventoryItemInstanceInterface>> GetAllItemInstances() const;

	/** Returns an item entry associated by its id. */
	FInventoryItemEntry* FindItemEntryById(const FInventoryItemId& ItemId) const;

	/** Returns the first item entry with matching item definition. */
	FInventoryItemEntry* FindFirstItemEntryByDefinition(const UItemDefinitionBase* ItemDefinition) const;

	/** Returns all replicated item instances. */
	const TArray<TObjectPtr<UObject>>& GetReplicatedItemInstances() const { return AllReplicatedItemInstances; }

	/** Returns the full list of all item entries in this inventory. */
	const FInventoryItemList& GetInventoryList() const { return InventoryList; }

	/** Called when an item was just given to the inventory. Used to initialize it. */
	UE_API virtual void OnGiveItem(FInventoryItemEntry& ItemEntry);

	/** Called when an item is about to be removed from the inventory. */
	UE_API virtual void OnRemoveItem(FInventoryItemEntry& ItemEntry);


	/** Notify that gets called whenever an item has changed, called by the inventory itself or due to replication. */
	UE_API virtual void NotifyItemChanged(const FInventoryItemEntry& ItemThatChanged, const int32& LastCount, const int32& NewCount) {}

	/** Notify that gets called whenever an item got added to the inventory, called by the inventory itself or due to replication. */
	UE_API virtual void NotifyItemAdded(const FInventoryItemEntry& ItemThatWasAdded, const int32& LastCount, const int32& NewCount) {}

	/** Notify that gets called whenever an item got removed to the inventory, called by the inventory itself or due to replication. */
	UE_API virtual void NotifyItemRemoved(const FInventoryItemEntry& ItemThatWasRemoved, const int32& LastCount, const int32& NewCount) {}

protected:
	//~ Begin UObject Interface
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	UE_API virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	//~ End UObject Interface

	//~ Begin AActor Interface
	UE_API virtual void PostInitializeComponents() override;
	//~ End AActor Interface

	/**
	 * Marks an item entry dirty for replication.
	 * bWasAddOrChange is an important flag to determine whether the entire array needs to be replicated,
	 * or if we can just replicate the item delta entry.
	 */
	UE_API void MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange = false, bool bForceMarkItemDirty = false);

	/** Checks whether we can create a new item stack for the given item entry. */
	UE_API virtual bool CanCreateNewStack(const FInventoryItemEntry& ItemEntry) const; 
	
	/** Constructs a new item entry but doesn't add to the inventory list. */
	UE_API FInventoryItemEntry CreateItemEntry(const UItemDefinitionBase* ItemDefinition, int32 StackCount, UObject* SourceObject) const;

	/** Processes a give item operation which is ensured to be valid. */
	UE_API virtual void ProcessGiveItemOperation(const TInventoryOpRef<FInventoryOp_GiveItem>& GiveOp);

	/** Processes a remove item operation which is ensured to be valid. */
	UE_API virtual void ProcessRemoveItemOperation(const TInventoryOpRef<FInventoryOp_RemoveItem>& RemoveOp);

	/** Processes the drop item method. */
	UE_API virtual AActor* DropItemImpl(FInventoryOp_RemoveItem::FParams& DropParams, const FInventoryItemEntry& ItemEntry);

	/** Checks whether the item entry matches the filter from the remove item params. */
	UE_API virtual bool MatchesRemoveFilter(const FInventoryItemEntry& ItemEntry, FInventoryOp_RemoveItem::FParams& Params) const;

	/** Combines two items. */
	UE_API virtual void CombineItems(FInventoryItemEntry& This, FInventoryItemEntry& Other, int32& OutCouldNotCombine);

	/** Attempts to create a new item stack in this inventory. */
	UE_API virtual bool AttemptCreateNewStack(const FInventoryItemEntry& ItemEntry, FInventoryItemId& OutItemId, const int32& RemainingStacks, int32& OutCreatedStackSize);

	/** Adds a new item instance to the replicated sub object list. */
	UE_API void AddReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance);

	/** Removes an item instance from the replicated sub object list. */
	UE_API void RemoveReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance);

	/** Creates the pickup creation data to use for spawning a pickup actor. */
	UE_API FPickupCreationData MakePickupCreationData(const FInventoryItemEntry& ItemEntry) const;
	
	/** Spawns a pickup actor using the given pickup creation data. */
	UE_API AActor* SpawnPickupActor(const FPickupCreationData& PickupCreationData) const;

	/** Called right after a pickup was spawned. */
	UE_API virtual void PostSpawnPickupActor(const FPickupCreationData& PickupCreationData, AActor* SpawnedPickup) const {}

	/** Returns the list of all item instances. */
	TArray<TObjectPtr<UObject>>& GetReplicatedItemInstances_Mutable() { return AllReplicatedItemInstances; }
	
public:
	/** Handle for outside inventory access. Gets set by the inventory component. */
	UPROPERTY()
	FInventoryHandle InventoryHandle;

protected:
	/** The type of inventory limit this inventory is limited to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Inventory)
	EInventoryLimitType InventoryLimitType = EInventoryLimitType::None;

	/** The inventory limit to use. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Inventory, meta=(EditCondition="InventoryLimitType!=EInventoryLimitType::None"))
	int64 InventoryLimit = 1;

	/** The pickup actor class to use when spawning pickups. Must implement the IItemPickupInterface. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Pickup, meta=(MustImplement="/Script/ItemizationCore.ItemPickupInterface"))
	TSubclassOf<AActor> PickupActorClass;
	
	/** Replicated list of inventory item entries. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing=OnRep_InventoryList, Category=Inventory)
	FInventoryItemList InventoryList;

	/** OnRep function that gets called whenever the InventoryList is replicated. */
	UFUNCTION()
	UE_API virtual void OnRep_InventoryList();

private:
	/** Full list of all replicated item instances that were added via an FInventoryItemEntry. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> AllReplicatedItemInstances;
	
	/** Timer handle for delayed removal of operations. */
	UFUNCTION()
	void FetchOpValidness();
	FTimerHandle FetchOpValidnessTimerHandle;

	/** List of pending and active requests. */
	UE::Itemization::FInventoryOpCache OpCache;

	friend class FGameplayDebuggerCategory_Itemization;
	TArray<TSharedPtr<FInventoryOp>> GetPendingOperations() const
	{
		return OpCache.GetPendingOperations();
	}
};

#undef UE_API
