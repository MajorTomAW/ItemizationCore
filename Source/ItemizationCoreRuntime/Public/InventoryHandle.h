// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryHandle.generated.h"

class AInventoryBase;

/** Unique identifier/handle that points to an inventory. */
USTRUCT(BlueprintType)
struct FInventoryHandle
{
	GENERATED_BODY()

	FInventoryHandle() = default;
	virtual ~FInventoryHandle() = default;

protected:
	/** Weak pointer to the inventory this handle points to. */
	UPROPERTY(Transient)
	TWeakObjectPtr<AInventoryBase> Inventory;

	/**	 Unique identifier for this handle. */
	UPROPERTY(Transient)
	FGuid Guid;

public:
	/** Returns the inventory this handle points to. */
	AInventoryBase* GetInventory() const
	{
		return Inventory.Get();
	}

	/** Assigns a new inventory to this handle. */
	FGuid AssignInventory(AInventoryBase* InInventory);

	/** Checks if this handle is valid. */
	inline bool IsValid() const
	{
		return Inventory.IsValid();
	}

	/** Resets this handle to an invalid state. */
	void Reset()
	{
		Inventory.Reset();
		Guid.Invalidate();
	}

	/** Compares this handle to another handle. */
	bool operator==(const FInventoryHandle& Other) const
	{
		return Guid == Other.Guid;
	}

	/** Archive operator for serialization. */
	friend FArchive& operator<<(FArchive& Ar, FInventoryHandle& InventoryHandle)
	{
		Ar << InventoryHandle.Guid;
		Ar << InventoryHandle.Inventory;
		return Ar;
	}
};