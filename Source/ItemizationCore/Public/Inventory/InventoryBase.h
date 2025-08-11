// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "InventoryHandle.h"
#include "Items/InventoryItemEntry.h"
#include "Items/ItemDefinitionBase.h"

#include "Operations/InventoryOp.h"
#include "Operations/InventoryOpCache.h"
#include "Operations/InventoryOp_ItemAction.h"
#include "Operations/InventoryOp_RemoveItem.h"

#include "InventoryBase.generated.h"

class IInventoryItemInstanceInterface;
struct FInventoryOp_ItemAction;
class UInventoryItemInstance;
struct FInventoryItemEntry;

#define MY_API ITEMIZATIONCORE_API

/** Inventory class that manages an inventory list. */
UCLASS(HideCategories=(Input,Movement,Collision,Rendering,Physics), BlueprintType, Blueprintable, MinimalAPI)
class AInventoryBase : public AActor
{
	GENERATED_BODY()

public:
	MY_API AInventoryBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~ Begin UObject Interface
	MY_API virtual void BeginPlay() override;
	MY_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	MY_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	MY_API virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	MY_API virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	//~ End UObject Interface

	//~ Begin AActor Interface
	MY_API virtual void PostInitializeComponents() override;
	//~ End AActor Interface

public:
	/** Helper struct for creating new item entries. */
	struct FCreateItemEntryParams
	{
		FCreateItemEntryParams() {}
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
	MY_API virtual FInventoryItemEntry CreateItemEntry(const FCreateItemEntryParams& InParams);
	
	/** Adds an item to the inventory. */
	MY_API virtual TInventoryOpPtr<FInventoryOp_ItemAction> GiveItem(FInventoryOp_ItemAction::FParams&& Params, const FCreateItemEntryParams& CreateItemParams);

	/** Removes an item from the inventory. */
	MY_API virtual TInventoryOpPtr<FInventoryOp_RemoveItem> RemoveItem(FInventoryOp_RemoveItem::FParams&& Params);

public:
	MY_API virtual FInventoryItemEntry* FindItemEntryByHandle(const FInventoryItemHandle& ItemHandle) const;
	
	MY_API virtual TScriptInterface<IInventoryItemInstanceInterface> FindItemInstanceByHandle(const FInventoryItemHandle& ItemHandle) const;
	
	template <class ItemInstanceType = UObject>
	ItemInstanceType* FindItemInstanceByHandle(const FInventoryItemHandle& ItemHandle) const
	{
		return Cast<ItemInstanceType>(FindItemInstanceByHandle(ItemHandle));
	}
	
	MY_API virtual void OnRemoveItem(FInventoryItemEntry& ItemEntry);
	MY_API virtual void OnGiveItem(FInventoryItemEntry& ItemEntry);

	/** Handle for outside inventory access. Gets set by the inventory component. */
	UPROPERTY()
	FInventoryHandle InventoryHandle;

	
	/** Returns the full list of all item instances. */
	MY_API const TArray<UObject*>& GetAllItemInstances() const { return AllItemInstances; }
	MY_API TArray<TScriptInterface<IInventoryItemInstanceInterface>> GetAllItemInstancesAsInterfaces() const;

	/** Returns the full list of all item entries in the inventory. */
	MY_API const FInventoryItemContainer& GetInventoryList() const { return InventoryList; }

	/** Returns the list of all operations that are currently inside the op cache. */
	MY_API uint32 GetNumPendingOperations() const { return OpCache.GetNumOps(); }

protected:
	/** Evaluates the given ItemEntry and checks if it can be added to the inventory. */
	MY_API virtual void EvaluateItemEntry(FInventoryOp_ItemAction::FParams& Params);

	/** Internal version of GiveItem. Don't call this directly. */
	MY_API virtual void NativeGiveItem(TInventoryOpRef<FInventoryOp_ItemAction> Op);

	/** Internal version of RemoveItem. Don't call this directly. */
	MY_API bool NativeRemoveItem(TInventoryOpRef<FInventoryOp_RemoveItem> Op, bool bRecursive = true);
	
	/** Checks whether two item entries can be merged in a single stack. */
	MY_API virtual bool CanMergeItems(const FInventoryItemEntry& ThisEntry, const FInventoryItemEntry& OtherEntry) const;

	/** Actually performs the merge action by merging 'ThisEntry' into 'OtherEntry'. */
	MY_API virtual void MergeItems(const FInventoryItemEntry& ThisEntry, FInventoryItemEntry& OtherEntry, int32& OutExcess) const;

	/** Checks whether an item can create an entire new stack in this inventory. Might be restricted by traits. */
	MY_API virtual bool CanCreateNewStack(const FInventoryOp_ItemAction::FParams& Params);

	/** Checks whether the item wants a new item instance. Some items don't require having one. */ 
	MY_API virtual bool ShouldCreateNewInstanceOfItem(const FInventoryItemEntry& ItemEntry) const;

	/** Actually creates the new instance and adds it to the tracking list. */
	MY_API virtual TScriptInterface<IInventoryItemInstanceInterface> CreateNewInstanceOfItem(FInventoryItemEntry& ItemEntry);

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
	TArray<TObjectPtr<UObject>> AllItemInstances;

	/** Adds a new item instance to the replicated sub object list. */
	void AddReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance);

	/** Removes an item instance from the replicated sub object list. */
	void RemoveReplicatedItemInstance(const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance);

	/**
	 * Called to mark an item entry dirty for replication.
	 * bWasAddOrChange is an important flag to determine whether the entire array needs to be replicated,
	 * or if we can just replicate the item delta entry.
	 * 
	 * @param ItemEntry		The item entry to mark dirty.
	 * @param bWasAddOrChange	True, if the item was added or changed. False, if the item was removed.
	 */
	MY_API void MarkItemEntryDirty(FInventoryItemEntry& ItemEntry, bool bWasAddOrChange = false);

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

#undef MY_API