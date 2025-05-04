// Author: Tom Werner (MajorT), 2025

#pragma once

#include "EquippableInventorySlotBinding.generated.h"

class UInputAction;

/** Describes a single slot binding that maps a slot id to an input action. */
USTRUCT(BlueprintType)
struct FEquippableInventorySlotBinding
{
	GENERATED_BODY()

public:
	/** The slot id this binding is for. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Binding)
	int64 SlotId = INDEX_NONE;

	/** The input action to be bound to the specified slot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Binding)
	TSoftObjectPtr<UInputAction> InputAction;
};