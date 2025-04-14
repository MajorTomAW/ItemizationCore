// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "EquippableInventorySlotBinding.generated.h"

class UInputAction;
class UObject;
struct FFrame;

/**
 * Struct holding data about the binding between an inventory slot and an input action.
 */
USTRUCT(BlueprintType, DisplayName="Equippable Slot Binding")
struct FEquippableInventorySlotBindingDefinition
{
	GENERATED_BODY()

	FEquippableInventorySlotBindingDefinition()
		: Slot(0)
	{
	}
	
public:
	/** The slot id of this binding. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	uint32 Slot;

	/** The input action to bind to this slot. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TSoftObjectPtr<UInputAction> InputAction;
};

/** Struct holding data about a single input binding for an inventory slot. */
struct FEquippableInventorySlotBinding
{
public:
	uint32 BindHandle = 0;
};