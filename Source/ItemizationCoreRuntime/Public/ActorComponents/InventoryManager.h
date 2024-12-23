// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "InventoryItemEntry.h"
#include "InventoryItemEntryHandle.h"
#include "ItemDefinition.h"
#include "Components/ActorComponent.h"
#include "InventoryManager.generated.h"


struct FItemActionContextData;
struct FItemActionContextData;

/**
 * Manages the inventory of an actor.
 */
UCLASS(ClassGroup=(Itemization), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class ITEMIZATIONCORERUNTIME_API UInventoryManager : public UActorComponent
{
	GENERATED_UCLASS_BODY()
	friend struct FInventoryItemEntry;
	friend struct FInventoryItemContainer;

	/** Static getter to find the inventory manager on an actor. */
	static UInventoryManager* GetInventoryManager(AActor* Actor);

	/** Returns true if this component's actor has authority. */
	virtual bool IsOwnerActorAuthoritative() const;
	
	/**
	 * Gives an item to the inventory.
	 * Will be ignored if the actor is not authoritative.
	 * 
	 * @param ItemEntry The entry of the item to give.
	 * @param ContextData The context of how the item is being given.
	 * @param Excess [OUT] The amount of items that could not be added to the inventory.
	 * @returns The handle to the item that was given. 
	 */
	FInventoryItemEntryHandle GiveItem(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& ContextData, int32& Excess);

	/**
	 * Grants an item definition or the inventory and returns its handle.
	 * Will be ignored if the actor is not authoritative.
	 *
	 * @param ItemDefinition The definition of the item to give.
	 * @param StackCount The number of items to give.
	 * @param OutExcess [OUT] The amount of items that could not be added to the inventory.
	 * @returns The handle to the item that was given.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Itemization Core", meta = (DisplayName = "Give Item", ScriptName = "GiveItem"))
	FInventoryItemEntryHandle K2_GiveItem(UItemDefinition* ItemDefinition, int32 StackCount, int32& OutExcess);

	/**
	 * Builds a simple FInventoryItemEntry from a definition and stack count.
	 */
	virtual FInventoryItemEntry BuildItemEntryFromDefinition(UItemDefinition* ItemDefinition, int32 StackCount, const FItemActionContextData& ContextData);

	/**
	 * Checks whether two item entries can be combined.
	 * In this context, ThisEntry will try to merge into OtherEntry.
	 * This will iterate through all item components to check for potential limitations.
	 * 
	 * @param ThisEntry The item entry that will try to merge into the other entry. 
	 * @param OtherEntry	The target item entry that will be modified.
	 * @param Context	The context of the item action.
	 * @param Excess	[OUT] The amount of items that could not be added to the inventory.
	 * @return True, if the items can be combined.
	 */
	virtual bool CombineItemEntries(const FInventoryItemEntry& ThisEntry, FInventoryItemEntry& OtherEntry, FItemActionContextData& Context, int32& Excess);

	/**
	 * Checks whether we can create a new stack of an item.
	 * This will iterate through all item components to check for potential limitations.
	 * 
	 * @param ItemEntry The item entry we want to create a new stack of.
	 * @param Context	The context of the item action.
	 * @return True, if we can create a new stack of the item.
	 */
	virtual bool CanCreateNewStack(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& Context) const;

	/**
	 * Evaluates the current context of an item entry and fills the context struct with information about the item.
	 * This will iterate through all item components and let them fill the context.
	 * 
	 * @param ItemEntry		The entry of the item to evaluate.
	 * @param OutContext	The context that will be filled with information about the item.
	 */
	virtual void EvaluateCurrentContext(const FInventoryItemEntry& ItemEntry, FItemActionContextData& OutContext) const;

	/** Called to mark that an item entry has been modified. */
	void MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange = false);

	/** Returns the list of all items in the inventory. */
	TArray<FInventoryItemEntry>& GetInventoryItems_Mutable()
	{
		return InventoryList.Items;
	}

	/** Returns the list of all items in the inventory. Read-only. */
	const TArray<FInventoryItemEntry>& GetInventoryItems() const
	{
		return InventoryList.Items;
	}

	/** Gets a list of all items in the inventory of the specified type. */
	void GetAllItemsOfType(TArray<FInventoryItemEntry*>* OutItems, const UItemDefinition* ItemDefinition);

	/** Returns an item entry from the inventory for a given handle. */
	FInventoryItemEntry* FindItemEntryFromHandle(FInventoryItemEntryHandle Handle, EConsiderPending ConsiderPending = EConsiderPending::PendingRemove) const;

	/** Returns the list of all items in the inventory. */
	TArray<FInventoryItemEntry>& GetInventoryList() { return InventoryList.Items; }

	/**
	 * Returns a list with all given item handles.
	 * @param OutHandles Array that will be filled with the item handles.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Itemization Core")
	void GetAllItemHandles(TArray<FInventoryItemEntryHandle>& OutHandles) const;

	//~ Begin UObject Interface
	virtual void PreNetReceive() override;
	virtual void BeginPlay() override;
	//~ End UObject Interface
	
	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	
	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const override;
	//~ End UActorComponent Interface

public:
	void IncrementInventoryListLock();
	void DecrementInventoryListLock();

	/** Catches the flags that indicate whether this component has network authority. */
	void CacheIsNetSimulated();

public:
	/** Wipes all items from the inventory. This will be ignored if the actor is not authoritative. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Itemization Core")
	void ClearAllItems();

protected:
	/** The replicated inventory list. */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryList, BlueprintReadOnly, Transient, Category = "Items")
	FInventoryItemContainer InventoryList;

	UFUNCTION()
	virtual void OnRep_InventoryList();

#if WITH_GAMEPLAY_MESSAGE_ROUTER
	/** Broadcasts an inventory change message via the gameplay message router. */
	virtual void BroadcastInventoryChangeMessage(UInventoryItemInstance* ItemThatChanged, int32 OldStackCount, int32 NewStackCount) const;
#endif

	FTimerHandle OnRep_InventoryListTimerHandle;

protected:
	/** Cached value of rather this is a simulated actor. */
	UPROPERTY()
	bool bCachedIsNetSimulated;
	
	/** Indicates how many levels of INVENTORY_SCOPE_LOCK() we're in. The inventory list may not be modified while InventoryScopeLockCount > 0. */
	int32 InventoryScopeLockCount;

	/**
	 * Whether all items should be removed when exiting the current inventory scope lock.
	 * Will be prioritized over pending Adds.
	 */
	bool bInventoryPendingClearAll;

	/** Items that will be removed when exiting the current inventory scope lock. */
	TArray<FInventoryItemEntryHandle, TInlineAllocator<2>> ItemPendingRemoves;

	/** Items that will be added when exiting the current inventory scope lock. */
	TArray<FInventoryItemEntry, TInlineAllocator<2>> ItemPendingAdds;

	/** Will be called from RemoveItem or from OnRep. */
	virtual void OnRemoveItem(FInventoryItemEntry& ItemEntry);

	/** Will be called from GiveItem or from OnRep. Initializes the given item instance. */
	virtual void OnGiveItem(FInventoryItemEntry& ItemEntry);

	/** Checks whether we need to create a new instance of an item, aka it doesn't stack, or the stack is full. */
	virtual bool ShouldCreateNewInstanceOfItem(const FInventoryItemEntry& ItemEntry, const uint32 InStackCount) const;

	/** Creates a new instance of an item, storing it in the spec. */
	virtual UInventoryItemInstance* CreateNewInstanceOfItem(FInventoryItemEntry& ItemEntry);

	/** Add a new item instance to the inventory. */
	void AddReplicatedItemInstance(UInventoryItemInstance* ItemInstance);

	/** Remove an item instance from the inventory. */
	void RemoveReplicatedItemInstance(UInventoryItemInstance* ItemInstance);

protected:
	struct FInventoryListLockActiveChange
	{
		FInventoryListLockActiveChange(
			UInventoryManager& InManager,
			TArray<FInventoryItemEntry, TInlineAllocator<2>>& PendingAdds,
			TArray<FInventoryItemEntryHandle, TInlineAllocator<2>>& PendingRemoves)
		
			: InventoryManager(InManager)
			, Adds(MoveTemp(PendingAdds))
			, Removes(MoveTemp(PendingRemoves))
		{
			InventoryManager.InventoryListLockActiveChanges.Add(this);
		}

		~FInventoryListLockActiveChange()
		{
			InventoryManager.InventoryListLockActiveChanges.Remove(this);
		}
		
		UInventoryManager& InventoryManager;
		TArray<FInventoryItemEntry, TInlineAllocator<2>> Adds;
		TArray<FInventoryItemEntryHandle, TInlineAllocator<2>> Removes;
	};

	TArray<FInventoryListLockActiveChange*> InventoryListLockActiveChanges;

public:
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
};
