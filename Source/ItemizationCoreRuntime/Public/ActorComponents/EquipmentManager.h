// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "InventoryEquipmentEntry.h"
#include "InventoryExtensionComponent.h"
#include "Components/ActorComponent.h"
#include "EquipmentManager.generated.h"

/**
 * Manages the equipment of an actor.
 */
UCLASS(ClassGroup = (Itemization), meta = (BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class ITEMIZATIONCORERUNTIME_API UEquipmentManager : public UInventoryExtensionComponent
{
	GENERATED_BODY()
	friend struct FInventoryEquipmentEntry;
	friend struct FInventoryEquipmentContainer;

public:
	UEquipmentManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Static getter to find the equipment manager on an actor. */
	UFUNCTION(BlueprintPure, Category = ItemizationCore)
	static UEquipmentManager* FindEquipmentManager(AActor* Actor);

	/**
	 * Equips an item that exists in the inventory.
	 * Will be ignored if the actor is not authoritative.
	 * @note This will first search for the item in the inventory manager before equipping it.
	 * 
	 * @param ItemHandle The handle of the item to equip. 
	 * @param ContextData	The context of how the item is being equipped.
	 * @returns The handle to the item that was equipped (In most cases this will be the same as the input handle).
	 */
	FInventoryItemEntryHandle EquipItem(const FInventoryItemEntryHandle& ItemHandle, const FItemActionContextData& ContextData);
	FInventoryItemEntryHandle EquipItem(const FInventoryItemEntryHandle& ItemHandle);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Itemization Core", meta = (DisplayName = "Equip Item", ScriptName = "EquipItem"))
	FInventoryItemEntryHandle K2_EquipItem(FInventoryItemEntryHandle ItemHandle);

	/**
	 * Unequips an item that is currently equipped.
	 * Will be ignored if the actor is not authoritative.
	 * @note This will first search for the item in the equipment list before unequipping it.
	 *
	 * @param ItemHandle The handle of the item to unequip.
	 * @param ContextData	The context of how the item is being unequipped.
	 * @returns True if the item was successfully unequipped.
	 */
	bool UnequipItem(const FInventoryItemEntryHandle& ItemHandle, const FItemActionContextData& ContextData);
	bool UnequipItem(const FInventoryItemEntryHandle& ItemHandle);

	/**
	 * Returns an item entry from the inventory for a given handle.
	 * @note Only works on the autonomous proxy!
	 */
	FInventoryItemEntry* FindItemEntryFromHandle(FInventoryItemEntryHandle Handle, EConsiderPending ConsiderPending = EConsiderPending::PendingRemove) const;

	/** Returns an equipment entry from the equipment list for a given handle. */
	FInventoryEquipmentEntry* FindEquipmentEntryFromHandle(FInventoryItemEntryHandle Handle) const;

	/** Returns the first equipped instance of a given type. */
	UInventoryEquipmentInstance* GetFirstInstanceOfType(TSubclassOf<UInventoryEquipmentInstance> InstanceType) const;
	template <class T = UInventoryEquipmentInstance>
	T* GetFirstInstanceOfType() const
	{
		return Cast<T>(GetFirstInstanceOfType(T::StaticClass()));
	}

	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const override;
	//~ End UActorComponent Interface

	//~ Begin UInventoryExtensionComponent Interface
	virtual void PreInitializeWithInventoryManager(UInventoryManager* InInventoryManager) override;
	//~ End UInventoryExtensionComponent Interface

protected:
	/** Will be called from EquipItem or from OnRep. Initializes the given equipment instance. */
	virtual void OnEquipItem(FInventoryEquipmentEntry& EquipmentEntry);

	/** Will be called from UnequipItem or from OnRep. */
	virtual void OnUnequipItem(FInventoryEquipmentEntry& EquipmentEntry);

	/** Called to mark that an equipment entry has been modified. */
	void MarkEquipmentEntryDirty(FInventoryEquipmentEntry& EquipmentEntry, bool bWasAddOrChange = false);

	/** Creates a new instance of equipment, storing it in the equipment entry. */
	virtual UInventoryEquipmentInstance* CreateNewInstanceOfEquipment(FInventoryEquipmentEntry& EquipmentEntry);

	/** Add a new item/Equipment instance to the replicated sub-object list. */
	void AddReplicatedInstance(UObject* Instance);

	/** Remove an item/Equipment instance from the replicated sub-object list. */
	void RemoveReplicatedInstance(UObject* Instance);

protected:
	/** The replicated equipment list. */
	UPROPERTY(Replicated, BlueprintReadOnly, Transient, Category = Equipment)
	FInventoryEquipmentContainer EquipmentList;
};
