// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemComponent.h"

#include "ItemDefinition.h"
#include "InventoryItemEntry.h"
#include "InventoryItemEntryHandle.h"

#include "InventoryManager.generated.h"

struct FItemActionContextData;

/**
 * Manages the inventory of an actor.
 * Items are defined by the UItemDefinition asset, which is const and can't be changed,
 * while the item instance is mutable and can be changed.
 *
 *
 * For each stack of items in the inventory system, there will be one item instance associated with its item definition.
 * The same thing for other data such as the stack count, its handle, and other data.
 *
 * Additionally, item definitions can have components that can be used to add custom logic to the item.
 */
UCLASS(ClassGroup = (Itemization), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class ITEMIZATIONCORERUNTIME_API UInventoryManager : public UActorComponent
{
	GENERATED_BODY()
	friend struct FInventoryItemEntry;
	friend struct FInventoryItemContainer;

public:
	UInventoryManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Static getter to find the inventory manager on an actor. */
	UFUNCTION(BlueprintPure, Category = ItemizationCore)
	static UInventoryManager* FindInventoryManager(AActor* Actor);

	/** Returns true if this component's actor has authority. */
	virtual bool IsOwnerActorAuthoritative() const;

	/**
	 * Forces the avatar actor to replicate.
	 * Useful for when the avatar actor is set after the inventory manager has been created.
	 */
	virtual void ForceAvatarReplication();
	
	/**
	 * Gives an item to the inventory.
	 * Will be ignored if the actor is not authoritative.
	 * 
	 * @param ItemEntry The entry of the item to give.
	 * @param ContextData The context of how the item is being given.
	 * @param Excess [OUT] The amount of items that couldn't be added to the inventory.
	 * @returns The handle to the item that was given. 
	 */
	FInventoryItemEntryHandle GiveItem(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& ContextData, int32& Excess);
	FInventoryItemEntryHandle GiveItem(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& ContextData);
	FInventoryItemEntryHandle GiveItem(const FInventoryItemEntry& ItemEntry, int32& Excess);
	FInventoryItemEntryHandle GiveItem(const FInventoryItemEntry& ItemEntry);

#if WITH_EQUIPMENT_SYSTEM
	FInventoryItemEntryHandle GiveAndEquipItem(const FInventoryItemEntry& ItemEntry, int32& Excess);
#endif

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
	 * Removes a given item from the inventory.
	 * @param Handle	The handle of the item to remove.
	 * @param StackCount	The number of items to remove. If -1, all items will be removed.
	 * @returns True, if the item was removed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Itemization Core")
	bool RemoveItem(const FInventoryItemEntryHandle& Handle, int32 StackCount = -1);

	/**
	 * Removes an item by its definition from the inventory.
	 * Meaning that all items with the same definition, no matter the handle, will be removed.
	 * Will be ignored if the actor is not authoritative.
	 *
	 * @param ItemDefinition The definition of the item to remove.
	 * @param StackCount The number of items to remove. If -1, all items will be removed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Itemization Core", meta = (DisplayName = "Remove Item By Definition", ScriptName = "RemoveItemByDefinition"))
	void K2_ConsumeItem(UItemDefinition* ItemDefinition, int32 StackCount = -1);

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
	 * @param Excess	[OUT] The amount of items that couldn't be added to the inventory.
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

	/** Returns an item entry from the inventory for a given definition. */
	FInventoryItemEntry* FindItemEntryFromDefinition(UItemDefinition* ItemDefinition) const;

	/** Returns the current stack count of an item in the inventory. */
	int32 GetCurrentStackCount(FInventoryItemEntryHandle Handle) const;

	/** Returns the list of all items in the inventory. */
	TArray<FInventoryItemEntry>& GetInventoryList() { return InventoryList.Items; }

	/** Creates a new item action context. */
	virtual FItemActionContextData CreateItemActionContextData(const FInventoryItemEntry& Item);

	/** Full list of all replicated item instances which are replicated to clients. */
	const TArray<UInventoryItemInstance*>& GetReplicatedItemInstances() const { return AllReplicatedItemInstances; }

	/** Returns the inventory data. */
	const TSharedPtr<FItemizationCoreInventoryData>& GetInventoryData() const { return InventoryData; }
	FItemizationCoreInventoryData* GetInventoryDataPtr() const { return InventoryData.Get(); }

	/**
	 * Returns a list with all given item handles.
	 * @param OutHandles Array that will be filled with the item handles.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Itemization Core")
	void GetAllItemHandles(TArray<FInventoryItemEntryHandle>& OutHandles) const;

	/** Returns the total number of all items in the inventory of the given type. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Itemization Core")
	int32 GetTotalItemCountByDefinition(const UItemDefinition* Type);

	/** Returns the total number of all items in the inventory of the given class. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Itemization Core")
	int32 GetTotalItemCountByClass(TSubclassOf<UItemDefinition> Type);

	/** Returns the total number of all items in the inventory of the given class exact. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Itemization Core")
	int32 GetTotalItemCountByClass_Exact(TSubclassOf<UItemDefinition> Type);

	//~ Begin UObject Interface
	virtual void PreNetReceive() override;
	virtual void BeginPlay() override;
	//~ End UObject Interface
	
	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemEntrySignature, FInventoryChangeMessage, Payload);

	/** Called when an item is added to the inventory. */
	UPROPERTY(BlueprintAssignable, Category=Inventory)
	FOnItemEntrySignature OnItemAddedDelegate;

	/** Called when an item is removed from the inventory. */
	UPROPERTY(BlueprintAssignable, Category=Inventory)
	FOnItemEntrySignature OnItemRemovedDelegate;

	/** Called when an item is changed in the inventory. */
	UPROPERTY(BlueprintAssignable, Category=Inventory)
	FOnItemEntrySignature OnItemChangedDelegate;

	FTimerHandle OnRep_InventoryListTimerHandle;

public:
	/** Returns the item added delegate. */
	FOnItemEntrySignature& GetOnItemAdded() { return OnItemAddedDelegate; }

	/** Returns the item removed delegate. */
	FOnItemEntrySignature& GetOnItemRemoved() { return OnItemRemovedDelegate; }

	/** Returns the item changed delegate. */
	FOnItemEntrySignature& GetOnItemChanged() { return OnItemChangedDelegate; }

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

	/** Creates a new instance of an item, storing it in the item entry. */
	virtual UInventoryItemInstance* CreateNewInstanceOfItem(FInventoryItemEntry& ItemEntry);

	/** Removes an item instance from the inventory that matches the predicate. */
	virtual bool RemoveItemByPredicate(const TFunctionRef<bool(const FInventoryItemEntry&)>& Predicate, int32 StackCount = -1);

	/** Add a new item instance to the inventory. */
	void AddReplicatedItemInstance(UInventoryItemInstance* ItemInstance);

	/** Remove an item instance from the inventory. */
	void RemoveReplicatedItemInstance(UInventoryItemInstance* ItemInstance);

	/** Native version of GiveItem(). DON'T call this directly. */
	virtual FInventoryItemEntryHandle NativeGiveItem(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& ContextData, int32& Excess);

	/** Returns the total number of all items in the inventory that match the predicate. */
	int32 GetTotalItemCountByPredicate(const TFunctionRef<bool(const FInventoryItemEntry&)>& Predicate);

private:
	/** The actor that owns this component logically. */
	UPROPERTY(ReplicatedUsing = OnRep_OwnerActor, Transient)
	TObjectPtr<AActor> OwnerActor;

	/** The actor that is the physical representation of the owner. */
	UPROPERTY(ReplicatedUsing = OnRep_OwnerActor, Transient)
	TObjectPtr<AActor> AvatarActor;

	/** Full list of all replicated item instances which are replicated to clients. */
	UPROPERTY()
	TArray<TObjectPtr<UInventoryItemInstance>> AllReplicatedItemInstances;

	/** Full-mutable list of all replicated item instances which are replicated to clients. */
	TArray<TObjectPtr<UInventoryItemInstance>>& GetReplicatedItemInstances_Mutable() { return AllReplicatedItemInstances; }

public:
	/** Sets the owner actor. */
	void SetOwnerActor(AActor* NewOwnerActor);
	AActor* GetOwnerActor() const { return OwnerActor.Get(); }

	/** Used to set the avatar actor directly, bypassing the automatic setting. */
	void SetAvatarActor_Direct(AActor* NewAvatarActor);
	AActor* GetAvatarActor_Direct() const { return AvatarActor.Get(); }

	/** Used to set the avatar actor. */
	void SetAvatarActor(AActor* NewAvatarActor);
	AActor* GetAvatarActor() const;

protected:
	/**
	 * Cached data about the inventory system such as the owner actor, avatar actor, etc.
	 * Utility-struct for easy access to those data.
	 */
	TSharedPtr<FItemizationCoreInventoryData> InventoryData;

	/**
	 * Initializes the inventory system with the owner actor, avatar actor, and inventory manager.
	 * Also validates the inventory data.
	 * Doesn't need to be called manually unless you want to control both the owner and avatar actor.
	 * @param InOwnerActor	The actor that owns the inventory (virtually).
	 * @param InAvatarActor		The physical representation of the owner actor (physically).
	 */
	virtual void InitInventorySystem(AActor* InOwnerActor, AActor* InAvatarActor);

	/**
	 * Clears out the inventory system and the inventory data.
	 */
	virtual void ClearInventoryData();

	/** Called when the inventory system has been initialized. */
	virtual void OnInventorySystemInitialized();

	virtual void RefreshInventoryData() {}

	/** OnRep for the owner actor. */
	UFUNCTION()
	virtual void OnRep_OwnerActor();

public:
	/** Called when the controller is set. */
	virtual void OnControllerSet() {}
	
	/** Called when the avatar actor is destroyed. */
	UFUNCTION()
	virtual void OnAvatarActorDestroyed(AActor* InActor);

	/** Called when the owner actor destroyed. */
	UFUNCTION()
	virtual void OnOwnerActorDestroyed(AActor* InActor);

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
	virtual void DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) const;
};
