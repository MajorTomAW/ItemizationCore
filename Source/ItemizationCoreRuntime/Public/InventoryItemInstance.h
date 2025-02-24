// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemEntry.h"
#include "InventoryItemEntryHandle.h"
#include "ItemDefinition.h"
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
	GENERATED_BODY()
	REPLICATED_BASE_CLASS(UInventoryItemInstance);

	friend class UInventoryManager;

public:
	UInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
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
	virtual void SetCurrentEntryInfo(const FInventoryItemEntryHandle InHandle, const FItemizationCoreInventoryData* InventoryData);

	/** Called when the item instance is added to an inventory. */
	virtual void OnAddedToInventory(const FInventoryItemEntry& ItemEntry, const FItemizationCoreInventoryData* InventoryData);

	/** Called when the item instance is removed from an inventory. */
	virtual void OnRemovedFromInventory(const FInventoryItemEntry& ItemEntry, const FItemizationCoreInventoryData* InventoryData);

	/** Called to inform the instance that the avatar actor has been set or changed. */
	virtual void OnAvatarSet(const FInventoryItemEntry& ItemEntry, const FItemizationCoreInventoryData* InventoryData);

	/** Gets the current item handle of the associated item entry. */
	FInventoryItemEntryHandle GetCurrentItemHandle() const;

	/** Retrieves the actual item entry of the associated item handle. */
	FInventoryItemEntry* GetCurrentItemEntry() const;

	/** Retrieves the item definition of the associated item entry. */
	UItemDefinition* GetCurrentItemDefinition() const;
	template <class T>
	T* GetCurrentItemDefinition() const
	{
		static_assert(TIsDerivedFrom<T, UItemDefinition>::IsDerived, "T must be a subclass of UItemDefinition");
		return Cast<T>(GetCurrentItemDefinition());
	}

	/** Gets the current inventory data associated with this instance. */
	const FItemizationCoreInventoryData* GetCurrentInventoryData() const;

	/** Gets the current item definition of the associated item entry. */
	EUserFacingItemState GetCurrentState() const { return CurrentState; }

	/** True if this is the server or single player. */
	bool HasAuthority() const;

	/** True if the owning actor is locally controlled, always true for single player. */
	bool IsLocallyControlled() const;

public:
	// ----------------------------------------------------
	// Utility functions for blueprints
	// ----------------------------------------------------

	/** Gets the current inventory manager that owns this item instance. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item")
	UInventoryManager* GetOwningInventoryManager() const;
	UInventoryManager* GetOwningInventoryManager_Checked() const;
	UInventoryManager* GetOwningInventoryManager_Ensured() const;
	
	/** Returns the inventory data associated with this instance. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item")
	FItemizationCoreInventoryData GetInventoryData() const;

	/** Returns the actor that owns this instance. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item")
	AActor* GetOwningActorFromInventoryData() const;

	/** Returns the avatar actor that represents the owner actor. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item")
	AActor* GetAvatarActorFromInventoryData() const;

	/** Retrieves the source object associated with this instance. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item")
	UObject* GetSourceObject() const;

	/** Returns the current stack count of the owning item entry. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item")
	int32 GetStackCount() const;
	
	/** Gets the current item handle of the associated item entry. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item", meta = (DisplayName = "Get Item Handle"))
	FInventoryItemEntryHandle K2_GetCurrentItemHandle() const { return GetCurrentItemHandle(); }

	/** Retrieves the actual item entry of the associated item handle. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item", meta = (DisplayName = "Get Item Entry"))
	FInventoryItemEntry& K2_GetCurrentItemEntry() const { return *GetCurrentItemEntry(); }

	/** Retrieves the item definition of the associated item entry. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item", meta = (DisplayName = "Get Item Definition"))
	const UItemDefinition* K2_GetCurrentItemDefinition() const { return GetCurrentItemDefinition(); }

	/** Retrieves the item typed definition of the associated item entry. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item", meta = (DisplayName = "Get Item Definition (Typed)", DeterminesOutputType = "Type"))
	const UItemDefinition* K2_GetCurrentItemDefinition_Typed(TSubclassOf<UItemDefinition> Type) const;

	/** Gets the current item definition of the associated item entry. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item", meta = (DisplayName = "Get Current State"))
	EUserFacingItemState K2_GetCurrentState() const { return GetCurrentState(); }

	/** True if this is the server or single player. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Item", meta = (DisplayName = "Has Authority", ScriptName = "HasAuthority"))
	bool K2_HasAuthority() const { return HasAuthority(); }

	/** True if the owning actor is locally controlled, always true for single player. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Itemization Core|Item", meta = (DisplayName = "Is Locally Controlled", ExpandBoolAsExecs = "ReturnValue"))
	bool K2_IsLocallyControlled() const { return IsLocallyControlled(); }

protected:
	/**
	 * Potential main entry point for subclasses to implement custom logic when the item is added to an inventory.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Itemization Core|Item", meta = (DisplayName = "On Added To Inventory", ScriptName = "OnAddedToInventory"))
	void K2_OnAddedToInventory();

	/**
	 * Potential main entry point for subclasses to implement custom logic when the item is removed from an inventory.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Itemization Core|Item", meta = (DisplayName = "On Removed From Inventory", ScriptName = "OnRemovedFromInventory"))
	void K2_OnRemovedFromInventory();

	bool bHasBlueprintAddedToInventory : 1;
	bool bHasBlueprintRemovedFromInventory : 1;

protected:
	/** A count of all current scope locks. */
	mutable int8 ScopeLockCount;

	/** Increases the scope lock count. */
	void IncrementListLock() const;

	/** Decreases the scope lock count. Runs the waiting to execute. */
	void DecrementListLock() const;

	/** Handle to the item entry that this instance is associated with. */
	mutable FInventoryItemEntryHandle CurrentEntryHandle;

	/** Shared cached data about the inventory system. */
	mutable const FItemizationCoreInventoryData* CurrentInventoryData;

	/** The current state of the item. */
	UPROPERTY()
	EUserFacingItemState CurrentState;
};
