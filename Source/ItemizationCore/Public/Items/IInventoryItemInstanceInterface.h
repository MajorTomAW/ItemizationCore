// Author: Tom Werner (MajorT), 2025

#pragma once

#include "UObject/Interface.h"

#include "IInventoryItemInstanceInterface.generated.h"

class UItemDefinitionBase;
struct FInventoryHandle;
struct FInventoryItemEntry;

#define UE_API ITEMIZATIONCORE_API

/** Interface for an object that can act as an item instance for an item entry in the inventory. */
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint), MinimalAPI)
class UInventoryItemInstanceInterface : public UInterface
{
	GENERATED_BODY()
};

/** Interface for an object that can act as an item instance for an item entry in the inventory. */
class IInventoryItemInstanceInterface
{
	GENERATED_BODY()

public:
	/** Called when the item instance is added to an inventory. */
	virtual void OnAddedToInventory(FInventoryItemEntry& ItemEntry, const FInventoryHandle& OwningInventoryHandle) = 0;

	/** Called right before the item instance is removed from an inventory. */
	virtual void OnRemovedFromInventory(FInventoryItemEntry& ItemEntry, const FInventoryHandle& OwningInventoryHandle) = 0;

	/** Checks whether this item can be combined with the given one. */
	virtual bool CanCombineWith(const FInventoryItemEntry& ItemEntry) const { return true; }

	/** Gets the net condition for this instance as a replicated subobject. */
	virtual ELifetimeCondition GetNetCondition() const { return COND_ReplayOrOwner; }

	/** Returns the source object that created this item instance, if any. */
	UFUNCTION(BlueprintCallable, Category=ItemInstance)
	virtual UObject* GetSourceObject() const = 0;

	/** Template function to cast the source object to a specific type. */
	template <class SourceObjectType = UObject>
	SourceObjectType* GetSourceObject() const
	{
		return Cast<SourceObjectType>(GetSourceObject());
	}

	/** Returns the item entry of this instance. */
	virtual FInventoryItemEntry* GetItemEntry() const = 0;

	/** Returns the item definition of this instance. */
	UFUNCTION(BlueprintCallable, Category=ItemInstance)
	UE_API virtual const UItemDefinitionBase* GetItemDefinition() const;

	/** Returns the typed item definition of this instance. */
	UFUNCTION(BlueprintCallable, Category=ItemInstance, meta=(DeterminesOutputType="ItemType"))
	UE_API virtual const UItemDefinitionBase* GetItemDefinition_Typed(TSubclassOf<UItemDefinitionBase> ItemType) const;
	template <class ItemType>
	const ItemType* GetItemDefinition_Typed() const
	{
		return Cast<const ItemType>(GetItemDefinition());
	}

	/** Returns if this item instance wants to be replicated. */
	UFUNCTION(BlueprintCallable, Category=ItemInstance)
	virtual bool GetIsReplicated() const { return false; };
};

#undef UE_API
