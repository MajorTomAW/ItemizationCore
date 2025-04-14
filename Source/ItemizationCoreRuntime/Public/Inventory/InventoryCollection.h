// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Items/InventoryItemEntry.h"

#include "InventoryCollection.generated.h"

enum class EItemizationInventoryType : uint8;
class UObject;
class AInfo;
class AActor;

/**
 * Inventory collection class that is used to manage a collection of inventories.
 * This will store multiple inventories and allow for easy access to them by other systems.
 * Potential use cases include static inventories for the world (e.g. chests, vendors, etc.)
 */
UCLASS(Blueprintable, BlueprintType, NotPlaceable)
class ITEMIZATIONCORERUNTIME_API AInventoryCollection : public AInfo
{
	GENERATED_BODY()

public:
	AInventoryCollection(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual ~AInventoryCollection() override;

	virtual bool CreateInventory(UObject* InOwner, FGuid& OutInventoryId, FInventoryItemContainer& OutInventory);

protected:
	void AddInventory_Internal(const FGuid& InventoryId, FInventoryItemContainer& Inventory);

protected:
	/** The name of the inventory collection. */
	UPROPERTY(Transient)
	TArray<FInventoryItemContainer> InventoryList;

	/** TMap for fast access to inventories by their GUID. */
	TMap<FGuid, FInventoryItemContainer*> InventoryMap;

	/** The type of inventory this collection is. */
	UPROPERTY()
	EItemizationInventoryType InventoryType;
};
