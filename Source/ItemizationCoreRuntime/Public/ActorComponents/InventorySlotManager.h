// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryExtensionComponent.h"
#include "InventorySlotEntry.h"
#include "Components/ActorComponent.h"

#include "InventorySlotManager.generated.h"

struct FItemizationCoreInventoryData;
struct FGameplayTag;
class UInventoryManager;
class UInventoryItemInstance;
class UItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSlotEntrySignature,
	UInventoryManager*, InInventoryManager,
	UInventorySlotManager*, InSlotManager,
	const FInventorySlotEntry&, InSlotEntry,
	const int32, SlotIndex);

/** A helper struct used to construct the default slots of an inventory. */
USTRUCT(BlueprintType)
struct FInventorySlotConstructor
{
	GENERATED_BODY()

public:
	FInventorySlotConstructor()
	{
	}

public:
	/** The slot tag that the created slot should have. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Slot)
	FGameplayTag SlotTag;

	/** The number of slots to create of this type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Slot)
	int32 NumSlots = 1;

	/** If false, this slot will never be shown in the UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Slot)
	bool bVisible = true;
};

/**
 * Manages the slots of an inventory.
 * This component is meant to be used in conjunction with the InventoryManager component. 
 * Slots are purely virtual and don't have any physical representation.
 * Also, they don't store any items,
 * rather they contain the cached-off handle to the item which lives in the inventory.
 *
 * Slots are primarily used to manage and organize items in the inventory grid.
 * By default, slots aren't limited to any specific item type and have to be configured via the item traits.
 *
 * Having slots fits for many different inventory systems, for example, a grid-based inventory system.
 *
 * @see UInventoryManager
 */
UCLASS(ClassGroup=(Itemization), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class ITEMIZATIONCORERUNTIME_API UInventorySlotManager : public UInventoryExtensionComponent
{
	GENERATED_BODY()
	friend class UInventoryManager;
	friend class UEquipmentManager;

public:
	UInventorySlotManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Static getter to find the slot manager on an actor. */
	UFUNCTION(BlueprintPure, Category = "Inventory|Slots", meta=(DefaultToSelf="Actor"))
	static UInventorySlotManager* FindInventorySlotManager(AActor* Actor);

	/** Attempts to find the given item handle and returns the slot id if found. */
	UFUNCTION(BlueprintPure, Category="Inventory|Slots")
	int32 FindSlotIdByHandle(const FInventoryItemEntryHandle& Handle, bool& OutSuccess) const;

	/** Returns the handle for a given slot id. */
	UFUNCTION(BlueprintPure, Category="Inventory|Slots")
	FInventoryItemEntryHandle FindHandleBySlotId(int32 SlotId, bool& OutSuccess) const;

	/** Tries to find a slot entry by its slot id. */
	UFUNCTION(BlueprintPure, Category="Inventory|Slots")
	void FindSlotById(int32 SlotId, bool& OutSuccess, FInventorySlotEntry& OutSlot) const;

	/** Tries to find the item instance of a slot by its slot id. */
	UFUNCTION(BlueprintPure, Category="Inventory|Slots")
	UInventoryItemInstance* FindItemInstanceBySlotId(int32 SlotId) const;

	/** Returns all slot entries. */
	UFUNCTION(BlueprintPure, Category = "Inventory|Slots")
	const TArray<FInventorySlotEntry>& GetSlotEntries() const;

public:
	/**
	 * Tries to add a specified item into a slot.
	 * Will check for permission and other constraints.
	 * 
	 * @param ItemHandle	The handle of the item to add.
	 * @param PreferredSlotId	The preferred slot id to add the item to. If INDEX_NONE, the system will find a suitable slot.
	 * @returns The actual slot id the item was added to. If INDEX_NONE, the item couldn't be added.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Slots")
	virtual int32 AddItemToSlot(const FInventoryItemEntryHandle& ItemHandle, int32 PreferredSlotId = -1);

	/**
	 * Removes an item from a slot.
	 * Clears out the handle that points to the item in the inventory.
	 * 
	 * @param ItemHandle	The handle of the item to remove. 
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Slots")
	virtual void RemoveItemFromSlot(const FInventoryItemEntryHandle& ItemHandle);

	/**
	 * Swaps two slots.
	 * Also works if one of the slots is empty.
	 * 
	 * @param SlotIdA	The slot id of the first slot. 
	 * @param SlotIdB	The slot id of the second slot.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Slots")
	virtual void SwapSlots(int32 SlotIdA, int32 SlotIdB);

	UFUNCTION(Server, Reliable)
	void Server_AddItemToSlot(const FInventoryItemEntryHandle& ItemHandle, int32 PreferredSlotId = INDEX_NONE);

	UFUNCTION(Server, Reliable)
	void Server_RemoveItemFromSlot(const FInventoryItemEntryHandle& ItemHandle);
	
	/** Tries to find a slot entry by its slot id. */
	const FInventorySlotEntry* FindSlotEntry(int32 SlotId) const;
	FInventorySlotEntry* FindSlotEntry(int32 SlotId);

	//~ Begin UActorComponent Interface
	virtual void InitializeComponent() override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const override;
	//~ End UActorComponent Interface

	UPROPERTY(BlueprintAssignable)
	FOnSlotEntrySignature OnSlotEntryAdded;

	UPROPERTY(BlueprintAssignable)
	FOnSlotEntrySignature OnSlotEntryRemoved;

	UPROPERTY(BlueprintAssignable)
	FOnSlotEntrySignature OnSlotEntryChanged;

private:
	int32 NativeAddItemToSlot(const FInventoryItemEntryHandle& ItemHandle, int32 PreferredSlotId = INDEX_NONE);
	void NativeRemoveItemFromSlot(const FInventoryItemEntryHandle& ItemHandle);

protected:
	/** The default slots to create. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Slots)
	TArray<FInventorySlotConstructor> DefaultSlots;

	/** The replicated list of slot entries. */
	UPROPERTY(Replicated)
	FInventorySlotContainer SlotContainer;
};
