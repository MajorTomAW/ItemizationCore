// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "InventoryDescriptor.generated.h"

class AInventory;
class AActor;
enum ENetRole : int;

/**
 * Struct describing the properties of an inventory.
 * Cached data for quick access such as the inventory owner, avatar, etc.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventoryDescriptorData
{
	GENERATED_BODY()
	virtual ~FInventoryDescriptorData() = default;

	FInventoryDescriptorData() = default;
	FInventoryDescriptorData(AActor* InOwner, AActor* InAvatar, AInventory* InInventory);

	/** Initializes this inventory descriptor data. */
	virtual void InitFromActor(AActor* InOwner, AActor* InAvatar = nullptr, AInventory* InInventory = nullptr);

	/** Updates this inventory descriptor data with a new avatar, in case the old one died and got respawned. */
	virtual void SetInventoryAvatar(AActor* InNewAvatar);

	/** Returns true if the inventory owner has net authority. */
	bool HasAuthority() const;

	/** Returns the local role of the inventory owner. */
	ENetRole GetLocalRole() const;

	/** Clears out the inventory descriptor data. */
	virtual void Reset();

public:
	/** The owning actor that owns this inventory. */
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TWeakObjectPtr<AActor> InventoryOwner;

	/** The avatar actor that physically represents this inventory's owner. */
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TWeakObjectPtr<AActor> InventoryAvatar;

	/** Optional player controller used for slot binding and input management. */
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TWeakObjectPtr<APlayerController> PlayerController;

	/** The inventory info object that actually owns this inventory. */
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TWeakObjectPtr<AInventory> OwningInventory;
};