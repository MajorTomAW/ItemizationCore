// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "Net/Core/PushModel/PushModelMacros.h"
#include "UObject/Object.h"

#include "InventoryItemHandle.h"
#include "ItemDefinition.h"

#include "InventoryItemInstance.generated.h"

class AInventoryBase;
class AInventory;
class UItemDefinition;
class FObjectInitializer;
class FLifetimeProperty;
class UWorld;
class UFunction;
class UObject;
class AActor;
struct FInventoryItemEntry;
struct FInventoryItemHandle;

/**
 * Instance of an item in an inventory.
 * Can be used to define custom gameplay logic.
 */
UCLASS(Blueprintable)
class ITEMIZATIONCORERUNTIME_API UInventoryItemInstance
	: public UObject
	, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	REPLICATED_BASE_CLASS(UInventoryItemInstance)
	friend class AInventory;
	friend struct FInventoryItemEntry;

public:
	UInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UObject Interface
	virtual UWorld* GetWorld() const override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack) override;
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitProperties() override;

#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

	//~ Begin IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~ End IGameplayTagAssetInterface

	/** Called to initialize this instance after being created due to replication. */
	virtual void PostNetInit() const;

	/** Called when this item instance is added to an inventory. */
	virtual void OnAddedToInventory(FInventoryItemEntry& ItemEntry, AInventory* OwningInventory);

	/** Called right before this item instance is removed from an inventory. */
	virtual void OnRemovedFromInventory(FInventoryItemEntry& ItemEntry, AInventory* OwningInventory);

public:
	/** Returns whether replication is enabled or not. */
	bool GetIsReplicated() const { return bReplicates; }

	/** Gets the owning inventory for this item instance. Will fall back to the outer of this object. */
	UFUNCTION(BlueprintCallable, Category = Item)
	AInventory* GetOwningInventory() const;

	/** Returns the item definition this item is representing. */
	UFUNCTION(BlueprintCallable, Category = Item)
	UItemDefinition* GetItemDefinition() const { return ItemDefinition; }

	/** Template function to cast the item definition to a specific type. */
	template <typename T>
	T* GetItemDefinition() const
	{
		return Cast<T>(GetItemDefinition());
	}

	/** Returns the typed item definition this item is representing. */
	UFUNCTION(BlueprintCallable, Category = Item, meta=(DeterminesOutputType=ItemClass))
	UItemDefinition* GetItemDefinition_Typed(TSubclassOf<UItemDefinition> ItemClass) const;

	/** Gets the current item entry associated with the item handle of this instance. */
	FInventoryItemEntry* GetItemEntry() const;

	/** Returns the source object that instigated the item instance creation. */
	UFUNCTION(BlueprintCallable, Category = Item)
	UObject* GetSourceObject() const;
	
	/** Template function to cast the source object to a specific type. */
	template <class T = UObject>
	T* GetSourceObject() const
	{
		return Cast<T>(GetSourceObject());
	}

protected:
	/** If true, the item instance will replicate to remote machines. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Replication)
	uint8 bReplicates : 1;

	/** Cached reference to the item definition this instance is representing. */
	UPROPERTY(BlueprintReadOnly, Category = Item, Transient)
	TObjectPtr<UItemDefinition> ItemDefinition;

	/** Cached reference to the inventory that this item instance is part of. This should usually be the same as the outer. */
	UPROPERTY(Transient)
	TWeakObjectPtr<AInventory> OwningInventoryPtr;

	/** Handle to the item entry that this instance is associated with. */
	mutable FInventoryItemHandle ItemHandle;
};
