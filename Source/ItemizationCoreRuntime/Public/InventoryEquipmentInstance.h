// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryEquipmentEntry.h"
#include "InventoryItemEntryHandle.h"
#include "InventoryItemInstance.h"
#include "Net/Core/PushModel/PushModelMacros.h"
#include "UObject/Object.h"
#include "InventoryEquipmentInstance.generated.h"

struct FInventoryEquipmentEntry;
struct FItemizationCoreInventoryData;

/**
 * Replicated equipment instance that can be used to define custom logic of equipment.
 */
UCLASS(Blueprintable)
class ITEMIZATIONCORERUNTIME_API UInventoryEquipmentInstance : public UObject
{
	GENERATED_BODY()
	REPLICATED_BASE_CLASS(UInventoryEquipmentInstance);

	friend class UEquipmentManager;

public:
	UInventoryEquipmentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UObject Interface
	virtual UWorld* GetWorld() const override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif
	//~	End UObject Interface

	/**
	 * True if this has been instanced, always true for blueprints.
	 * This will return always true by default as non-instanced equipment objects aren't supported yet.
	 */
	bool IsInstantiated() const;

	/** Called to initialize the current entry handle. */
	virtual void SetCurrentEntryInfo(const FInventoryItemEntryHandle InHandle, const FItemizationCoreInventoryData* InventoryData);

	/** Called when the equipment instance is added as equipment. */
	virtual void OnEquipped(const FInventoryEquipmentEntry& EquipmentEntry, const FItemizationCoreInventoryData* InventoryData);

	/** Called when the equipment instance is removed from equipment. */
	virtual void OnUnequipped(const FInventoryEquipmentEntry& EquipmentEntry, const FItemizationCoreInventoryData* InventoryData);

	/** Gets the current item handle of the associated equipment entry. */
	FInventoryItemEntryHandle GetCurrentEntryHandle() const;

	/** Retrieves the actual equipment entry of the associated equipment handle. */
	FInventoryEquipmentEntry* GetCurrentEquipmentEntry() const;

	/** Retrieves the item definition of the associated equipment entry. */
	UItemDefinition* GetCurrentItemDefinition() const;

	/** Gets the current inventory data associated with this instance. */
	const FItemizationCoreInventoryData* GetCurrentInventoryData() const;

	/** True if this is the server or single player. */
	bool HasAuthority() const;
	
	/**
	 * Called to inform the instance that the avatar actor has been set or changed.
	 * Here we will also spawn the equipment actors if any.
	 */
	virtual void OnAvatarSet(const FInventoryEquipmentEntry& EquipmentEntry, const FItemizationCoreInventoryData* InventoryData);

	// ----------------------------------------------------
	// Utility functions for blueprints
	// ----------------------------------------------------

	/** Gets the current equipment manager that owns this equipment instance. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Equipment")
	UEquipmentManager* GetOwningEquipmentManager() const;
	UEquipmentManager* GetOwningEquipmentManager_Checked() const;
	UEquipmentManager* GetOwningEquipmentManager_Ensured() const;

	/** Retrieves the typed instigator of this equipment (Usually the item instance). */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Equipment", meta = (DeterminesOutputType = "Type"))
	UObject* GetInstigatorTyped(TSubclassOf<UInventoryItemInstance> Type) const;
	
	/** Retrieves the instigator of this equipment (Usually the item instance). */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Equipment")
	UObject* GetInstigator() const { return GetInstigatorTyped(UInventoryItemInstance::StaticClass()); }

	/** returns the inventory data associated with this instance. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Equipment")
	FItemizationCoreInventoryData GetInventoryData() const;

	/** Gets the current item handle of the associated equipment/item entry. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Equipment", meta = (DisplayName = "Get Item Handle", ScriptName = "GetItemHandle"))
	FInventoryItemEntryHandle K2_GetCurrentItemHandle() const { return GetCurrentEntryHandle(); }

	/** Retrieves the actual equipment entry of the associated equipment handle. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Equipment", meta = (DisplayName = "Get Equipment Entry", ScriptName = "GetEquipmentEntry"))
	FInventoryEquipmentEntry& K2_GetCurrentEquipmentEntry() const { return *GetCurrentEquipmentEntry(); }

	/** retrieves the item definition of the associated equipment entry. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Equipment", meta = (DisplayName = "Get Item Definition", ScriptName = "GetItemDefinition"))
	UItemDefinition* K2_GetCurrentItemDefinition() const { return GetCurrentItemDefinition(); }

	/** True if this is the server or single player. */
	UFUNCTION(BlueprintCallable, Category = "Itemization Core|Equipment", meta = (DisplayName = "Has Authority", ScriptName = "HasAuthority"))
	bool K2_HasAuthority() const { return HasAuthority(); }

protected:
	/**
	 * Potential main entry point for subclasses to implement custom logic,
	 * when the equipment is added to an inventory and its avatar actor is set.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Itemization Core|Equipment", meta = (DisplayName = "On Equipped", ScriptName = "OnEquipped"))
	void K2_OnEquipped(AActor* Avatar);

	/**
	 * Called when this instance has been unequipped from the avatar actor.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Itemization Core|Equipment", meta = (DisplayName = "On Unequipped", ScriptName = "OnUnequipped"))
	void K2_OnUnequipped(AActor* Avatar);
	
	bool bHasBlueprintEquipped;
	bool bHasBlueprintUnequipped;

protected:
	/** Handle to the item entry that this instance is associated with. */
	mutable FInventoryItemEntryHandle CurrentEntryHandle;

	/** Shared cached data about the inventory system. */
	mutable const FItemizationCoreInventoryData* CurrentInventoryData;
};

