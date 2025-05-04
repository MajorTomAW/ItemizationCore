// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryItemHandle.h"
#include "InventorySlotHandle.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventoryItemSlot.generated.h"

/** Fast array serializer item for a single item slot in an inventory. */
USTRUCT(BlueprintType)
struct FInventoryItemSlot : public FFastArraySerializerItem
{
	GENERATED_BODY()
	friend struct FInventorySlotContainer;
	
public:
	UPROPERTY()
	FGameplayTag GroupTag;

	UPROPERTY()
	FGameplayTagContainer SlotTags;

	UPROPERTY()
	FInventorySlotHandle SlotHandle;

	UPROPERTY()
	FInventoryItemHandle ItemHandle;
};

USTRUCT()
struct FInventoryItemSlotGroup
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayTag GroupTag;

	UPROPERTY()
	TArray<FInventoryItemSlot> SlotList;
};

/** Fast array serializer for a list of item slots in an inventory. */
USTRUCT(BlueprintType)
struct FInventorySlotContainer : public FFastArraySerializer
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TArray<FInventoryItemSlot> ItemSlots;
};
