// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemEntry.h"
#include "InventoryItemEntryHandle.h"
#include "ItemizationCoreTypes.h"
#include "Net/Core/PushModel/PushModelMacros.h"
#include "UObject/Object.h"
#include "InventoryItemInstance.generated.h"

class UItemDefinition;
struct FInventoryItemEntry;

/**
 * Replicated item instance that can be used to define custom logic of an item.
 * With this, we always have access to the current stack count, the item definition, and other useful data.
 *
 * Can be extended to add custom logic for items.
 */
UCLASS(Blueprintable)
class ITEMIZATIONCORERUNTIME_API UInventoryItemInstance : public UObject
{
	GENERATED_UCLASS_BODY()
	REPLICATED_BASE_CLASS(UInventoryItemInstance);

	friend class UInventoryManager;

public:
	//~ Begin UObject Interface
	virtual UWorld* GetWorld() const override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack) override;
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif
	//~ End UObject Interface

	/** True if this has been instanced, always true for blueprints. */
	bool IsInstantiated() const;

	/** Called to initialize after being created due to replication */
	virtual void PostNetInit();

	/** Called to initialize the current entry handle. */
	virtual void SetCurrentEntryInfo(UInventoryManager* InventoryManager, const FInventoryItemEntryHandle& InHandle);

	/** Called when the item instance is added to an inventory. */
	virtual void OnAddedToInventory(UInventoryManager* InventoryManager, const FInventoryItemEntry& ItemEntry);

	/** Called when the item instance is removed from an inventory. */
	virtual void OnRemovedFromInventory(UInventoryManager* InventoryManager, const FInventoryItemEntry& ItemEntry);

	/** Called to inform the instance that the avatar actor has been set or changed. */
	virtual void OnAvatarSet(AActor* Avatar, const FInventoryItemEntry& ItemEntry);

	/** Gets the current item handle of the associated item entry. */
	FInventoryItemEntryHandle GetCurrentItemHandle() const;

	/** Retrieves the actual item entry of the associated item handle. */
	FInventoryItemEntry* GetCurrentItemEntry() const;

	/** Gets the current inventory manager that owns this item instance. */
	UInventoryManager* GetCurrentInventoryManager() const { return CurrentInventoryManager.Get(); }

protected:
	/** A count of all current scope locks. */
	mutable int8 ScopeLockCount;

	/** Handle to the item entry that this instance is associated with. */
	mutable FInventoryItemEntryHandle CurrentEntryHandle;

	/** Cached pointer to the inventory manager that owns this item instance. */
	TWeakObjectPtr<UInventoryManager> CurrentInventoryManager;

	/** The current state of the item. */
	UPROPERTY()
	ECurrentItemState CurrentState;
};
