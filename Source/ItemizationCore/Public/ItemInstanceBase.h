// Author: Tom Werner (dc: majort), 2026

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemId.h"
#include "UObject/Object.h"

#include "ItemInstanceBase.generated.h"

class UItemDefinitionBase;
class AInventoryBase;
struct FInventoryItemEntry;

#define UE_API ITEMIZATIONCORE_API

/**
 * An instance of a runtime item in an inventory.
 */
UCLASS(MinimalAPI)
class UItemInstanceBase : public UObject
{
	GENERATED_BODY()

public:
	UE_API UItemInstanceBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Called when the item was added to an inventory. */
	UE_API virtual void OnAddedToInventory(FInventoryItemEntry& OwningItemEntry, AInventoryBase* Inventory);

	/** Called when the item was removed from the inventory. */
	UE_API virtual void OnRemovedInventory(FInventoryItemEntry& OwningItemEntry, AInventoryBase* Inventory);

	/** Checks whether this item can be combined with the given one. */
	virtual bool CanCombineWith(const FInventoryItemEntry& ItemEntry) const { return true; }

	/** Gets the owning inventory by checking this items outer. */
	UFUNCTION(BlueprintPure, Category=Item)
	UE_API AInventoryBase* GetInventory() const;

	/** Gets the owning controller by looking at the owning inventory's owner. */
	UFUNCTION(BlueprintPure, Category=Item)
	UE_API APlayerController* GetOwningPlayer() const;

	/** Gets the owning item id. */
	UFUNCTION(BlueprintPure, Category=Item)
	FInventoryItemId GetItemId() const { return OwningItemId; }

	/** Gets the owning item definition. */
	UFUNCTION(BlueprintPure, Category=Item)
	const UItemDefinitionBase* GetItemDefinition() const { return ItemDefinition; }

	/** Gets the current stack size. */
	UFUNCTION(BlueprintPure, Category=Item)
	UE_API int32 GetStackSize() const;

	/** Gets the current durability of the item. */
	UFUNCTION(BlueprintPure, Category=Item)
	UE_API float GetDurability() const;

	/** Returns true if this item has durability. */
	UFUNCTION(BlueprintPure, Category=Item)
	UE_API bool HasDurability() const;

	/** Gets the item entry. */
	UE_API const FInventoryItemEntry* GetItemEntry() const;

	/** Returns true if this item is a server instance. */
	UE_API bool HasAuthority() const;

	/** Removes this item from its owning inventory. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Item)
	UE_API virtual void RemoveFromInventory();

	/** Checks whether this item can be dropped. */
	UFUNCTION(BlueprintPure, Category=Item)
	UE_API virtual bool CanBeDropped() const;

	/** Drops this item to the floor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Item)
	UE_API virtual AActor* DropItem();

protected:
	/** Called when the item was added to an inventory. */
	UFUNCTION(BlueprintImplementableEvent, Category=Item, DisplayName="On Added To Inventory")
	void OnAddedToInventoryBP(AInventoryBase* Inventory);
	bool bHasBlueprintOnAddedToInventory;

protected:
	//~ Begin UObject Interface
	UE_API virtual UWorld* GetWorld() const override;
	UE_API virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	UE_API virtual bool CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack) override;
	UE_API virtual bool IsSupportedForNetworking() const override { return true; }
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void PostInitProperties() override;

#if UE_WITH_IRIS
	UE_API virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif

#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

protected:
	/** Item entry that this instance is associated with. */
	UPROPERTY(Replicated)
	FInventoryItemId OwningItemId;

	/** Item Definition this item represents. Gets resolved when added to the inventory server- and client-side. */
	UPROPERTY(Replicated)
	TObjectPtr<const UItemDefinitionBase> ItemDefinition;
};

#undef UE_API
