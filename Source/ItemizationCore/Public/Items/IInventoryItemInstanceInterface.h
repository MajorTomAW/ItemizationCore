// Author: Tom Werner (MajorT), 2025

#pragma once

#include "UObject/Interface.h"

#include "IInventoryItemInstanceInterface.generated.h"

struct FInventoryHandle;
struct FInventoryItemEntry;

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

	/** Returns the source object that created this item instance, if any. */
	virtual UObject* GetSourceObject() const = 0;

	/** Template function to cast the source object to a specific type. */
	template <class SourceObjectType = UObject>
	SourceObjectType* GetSourceObject() const
	{
		return Cast<SourceObjectType>(GetSourceObject());
	}

	/** Returns if this item instance wants to be replicated. */
	virtual bool GetIsReplicated() const { return false; };
};
