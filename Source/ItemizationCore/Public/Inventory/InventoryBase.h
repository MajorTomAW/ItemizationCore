// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "InventoryHandle.h"
#include "Items/InventoryItemEntry.h"
#include "Items/InventoryItemSlot.h"
#include "Items/ItemDefinitionBase.h"

#include "Operations/InventoryOp.h"
#include "Operations/InventoryOpCache.h"
#include "Operations/InventoryOp_GiveAction.h"
#include "Operations/InventoryOp_RemoveItem.h"

#include "InventoryBase.generated.h"

struct FInventoryOp_PlaceItemInSlot;
class UInventoryConfigAsset;
class IInventoryItemInstanceInterface;
struct FInventoryOp_GiveAction;
class UInventoryItemInstance;
struct FInventoryItemEntry;

#define UE_API ITEMIZATIONCORE_API

/** Inventory class that manages an inventory list. */
UCLASS(HideCategories=(Input,Movement,Collision,Rendering,Physics), BlueprintType, Blueprintable, MinimalAPI)
class AInventoryBase : public AActor
{
	GENERATED_BODY()

public:
	UE_API AInventoryBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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

public:
	/** Helper struct for creating new item entries. */
	struct FCreateItemEntryParams
	{
		FCreateItemEntryParams() = delete;
		explicit FCreateItemEntryParams(UItemDefinitionBase* InItemDefinition, int32 InStackSize = 1, UObject* InSourceObject = nullptr)
			: ItemDefinition(InItemDefinition)
			, StackSize(InStackSize)
			, SourceObject(InSourceObject)
		{
		}

		TWeakObjectPtr<UItemDefinitionBase> ItemDefinition;
		int32 StackSize = 0;
		TWeakObjectPtr<UObject> SourceObject = nullptr;
	};

	/** Constructs a new inventory item entry. */
	UE_API virtual FInventoryItemEntry CreateItemEntry(const FCreateItemEntryParams& InParams);

	/** Adds an item to the inventory. */
	UE_API virtual TInventoryOpPtr<FInventoryOp_GiveAction> GiveItem(FInventoryOp_GiveAction::FParams&& Params, const FCreateItemEntryParams& CreateItemParams);

	/** Removes an item from the inventory. */
	UE_API virtual TInventoryOpPtr<FInventoryOp_RemoveItem> RemoveItem(FInventoryOp_RemoveItem::FParams&& Params);

	/** Assigns an unoccupied item slot to the given item handle. */
	UE_API virtual TInventoryOpPtr<FInventoryOp_PlaceItemInSlot> PlaceItemInSlot(FInventoryItemEntry& ItemEntry, FInventoryItemSlot& Slot);

public:
	UE_API FInventoryItemEntry* FindItemEntryByHandle(const FInventoryItemHandle& ItemHandle) const;
	UE_API FInventoryItemSlot* FindItemSlotByHandle(const FInventorySlotHandle& SlotHandle) const;

	UE_API virtual TScriptInterface<IInventoryItemInstanceInterface> FindItemInstanceByHandle(const FInventoryItemHandle& ItemHandle) const;

	template <class ItemInstanceType = UObject>
	ItemInstanceType* FindItemInstanceByHandle(const FInventoryItemHandle& ItemHandle) const
	{
		return Cast<ItemInstanceType>(FindItemInstanceByHandle(ItemHandle));
	}

	UE_API virtual TArray<TScriptInterface<IInventoryItemInstanceInterface>> GetItemInstancesInGroup(const FGameplayTag& InGroup);

	UE_API virtual void OnRemoveItem(FInventoryItemEntry& ItemEntry);
	UE_API virtual void OnGiveItem(FInventoryItemEntry& ItemEntry);

	/** Tries to find the next unoccupied item slot. */
	UE_API FInventoryItemSlot* GetNextUnoccupiedItemSlot(const FGameplayTag& InGroupTag) const;

	/** Handle for outside inventory access. Gets set by the inventory component. */
	UPROPERTY()
	FInventoryHandle InventoryHandle;


	/** Returns the full list of all item instances. */
	UE_API const TArray<UObject*>& GetAllItemInstances() const { return AllItemInstances; }
	UE_API TArray<TScriptInterface<IInventoryItemInstanceInterface>> GetAllItemInstancesAsInterfaces() const;

	/** Returns the full list of all item entries in the inventory. */
	UE_API const FInventoryItemContainer& GetInventoryList() const { return InventoryList; }

	/** Returns the full list of all item slots in the inventory. */
	UE_API const FInventorySlotContainer& GetSlotList() const { return InventorySlots; }

	/** Returns the list of all operations that are currently inside the op cache. */
	UE_API uint32 GetNumPendingOperations() const { return OpCache.GetNumOps(); }

	/** Initializes this inventory slots with the given inventory config. */
	UE_API void InitializeInventorySlots(const UInventoryConfigAsset* InventoryConfig);

protected:
	/** Evaluates the given ItemEntry and checks if it can be added to the inventory. */
	UE_API virtual void EvaluateItemEntry(FInventoryOp_GiveAction::FParams& Params);

	/** Internal version of GiveItem. Don't call this directly. */
	UE_API virtual void NativeGiveItem(const TInventoryOpRef<FInventoryOp_GiveAction>& Op);

	/** Internal version of RemoveItem. Don't call this directly. */
	UE_API bool NativeRemoveItem(const TInventoryOpRef<FInventoryOp_RemoveItem>& Op, bool bRecursive = true);

	/** Internal version of PlaceItemInSLot. Don't call this directly. */
	UE_API virtual void NativePlaceItemInSlot(const TInventoryOpRef<FInventoryOp_PlaceItemInSlot>& Op);

	/** Checks whether two item entries can be merged in a single stack. */
	UE_API virtual bool CanMergeItems(const FInventoryItemEntry& ThisEntry, const FInventoryItemEntry& OtherEntry) const;

	/** Actually performs the merge action by merging 'ThisEntry' into 'OtherEntry'. */
	UE_API virtual void MergeItems(const FInventoryItemEntry& ThisEntry, FInventoryItemEntry& OtherEntry, int32& OutExcess) const;

	/** Checks whether an item can create an entire new stack in this inventory. Might be restricted by traits. */
	UE_API virtual bool CanCreateNewStack(const FInventoryOp_GiveAction::FParams& Params) const;

	/** Checks whether an item can be combined with an existing stack to fill larger stacks first. */
	UE_API virtual bool CanAutoCombineStacks(const UItemDefinitionBase* ItemDefinition) const;

	/** Checks whether an item can be placed in an item slot. */
	UE_API virtual bool CanPlaceItemInSlot(const UItemDefinitionBase* ItemDefinition, const FInventoryItemSlot& Slot) const;

	/** Checks whether the item wants a new item instance. Some items don't require having one. */
	UE_API virtual bool ShouldCreateNewInstanceOfItem(const FInventoryItemEntry& ItemEntry) const;

	/** Actually creates the new instance and adds it to the tracking list. */
	UE_API virtual TScriptInterface<IInventoryItemInstanceInterface> CreateNewInstanceOfItem(FInventoryItemEntry& ItemEntry);

	UE_API virtual void NotifyItemAdded(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);
	UE_API virtual void NotifyItemRemoved(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);
	UE_API virtual void NotifyItemChanged(const FInventoryItemEntry& ItemEntry, const int32& LastCount, const int32& NewCount);


protected:
	/** Replicated list of inventory item entries. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing=OnRep_InventoryList, Category=Inventory)
	FInventoryItemContainer InventoryList;

	/** Replicated list of inventory slots. */
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing=OnRep_InventorySlots, Category=Inventory)
	FInventorySlotContainer InventorySlots;

	/** OnRep function that gets called whenever the InventoryList is replicated. */
	UFUNCTION()
	UE_API virtual void OnRep_InventoryList();
	FTimerHandle OnRep_InventoryListTimerHandle;

	/** OnRep function that gets called whenever the InventorySlots are replicated. */
	UFUNCTION()
	UE_API virtual void OnRep_InventorySlots();

	/** Full list of all item instances that were added via an FInventoryItemEntry. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> AllItemInstances;

	/** Adds a new item instance to the replicated sub object list. */
	void AddReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance);

	/** Removes an item instance from the replicated sub object list. */
	void RemoveReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance);

	/*UE_API void MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange = false);*/

	/**
	 * Called to mark an item entry dirty for replication.
	 * bWasAddOrChange is an important flag to determine whether the entire array needs to be replicated,
	 * or if we can just replicate the item delta entry.
	 *
	 * @param InList The list that owns the list entry
	 * @param InListEntry		The item entry to mark dirty.
	 * @param bWasAddOrChange	True, if the item was added or changed. False, if the item was removed.
	 * @param bForceMarkItemDirty If true, will force MarkItemDirty() over MarkArrayDirty() on authority.
	 */
	template <typename List, typename ListEntry>
	void MarkInventoryDirty(List& InList, ListEntry& InListEntry, bool bWasAddOrChange = false, bool bForceMarkItemDirty = false)
	{
		if (Owner->HasAuthority())
		{
			if (bWasAddOrChange || bForceMarkItemDirty)
			{
				InList.MarkItemDirty(InListEntry);
			}
			else
			{
				InList.MarkArrayDirty();
			}
		}
		else
		{
			// Client-side. mark the entire array dirty so it will be replicated
			InList.MarkArrayDirty();
		}
	}

	/** Timer handle for delayed removal of operations. */
	UFUNCTION()
	void FetchOpValidness();
	FTimerHandle FetchOpValidnessTimerHandle;

private:
	/** Returns the mutable full list of all item instances. */
	TArray<TObjectPtr<UObject>>& GetAllItemInstances_Mutable() { return AllItemInstances; }

	/** List of pending and active requests. */
	UE::Itemization::FInventoryOpCache OpCache;

	friend class FGameplayDebuggerCategory_Itemization;
	TArray<TSharedPtr<FInventoryOp>> GetPendingOperations() const
	{
		return OpCache.GetPendingOperations();
	}
};

#undef UE_API
