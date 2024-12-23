// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryItemEntry.h"

#include "ItemizationCoreTypes.generated.h"

class UInventoryManager;
struct FInventoryItemEntry;
class UItemDefinition;
class UInventoryItemInstance;

/**
 * Utility-struct that holds the data of an item actor that will be spawned in the world.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FItemActorData
{
	GENERATED_BODY()

public:
	FItemActorData()
	{}

	/** The actor blueprint that will be used to spawn this item definition. */
	UPROPERTY(EditAnywhere, Category = "Actor Data")
	TSoftClassPtr<AActor> ActorBlueprint;

	/** The relative transform of the actor blueprint. */
	UPROPERTY(EditAnywhere, Category = "Actor Data")
	FTransform RelativeTransform = FTransform::Identity;

	/** The influence radius. */
	UPROPERTY(EditAnywhere, Category = "Actor Data")
	float InfluenceDistance = 0.0f;

	/** The actor template id. */
	UPROPERTY(EditAnywhere, Category = "Actor Data")
	int32 ActorTemplateID = INDEX_NONE;
};

struct FItemActionContextData
{
	FItemActionContextData()
		: Instigator(nullptr)
		, InventoryManager(nullptr)
		, ContextTags(nullptr)
		, Delta(0)
		, SlotSize(1)
		, MaxStackSize(1)
	{
	}
	
	FItemActionContextData(const FInventoryItemEntry& ItemEntry)
	{
		Instigator = ItemEntry.SourceObject;
		InventoryManager = nullptr;
		ContextTags = nullptr;

		Delta = ItemEntry.StackCount;
		SlotSize = 1;
		MaxStackSize = 1;
	}
	
public:
	TWeakObjectPtr<UObject> Instigator; // The object that instigated the item.
	TWeakObjectPtr<UInventoryManager> InventoryManager; // Inventory manager that is handling the item.
	FGameplayTagContainer* ContextTags; // Option context tags.

	int32 Delta; // Delta count of items that should be granted.
	int32 SlotSize; // Number of slots that the item should occupy.
	int32 MaxStackSize; // Max number of items in a single stack.

public:
	FORCEINLINE bool IsValid() const
	{
		return InventoryManager.IsValid();
	}

	FORCEINLINE bool operator==(const FItemActionContextData& B) const
	{
		return Instigator == B.Instigator
			&& InventoryManager == B.InventoryManager
			&& ContextTags == B.ContextTags
			&& Delta == B.Delta
			&& SlotSize == B.SlotSize
			&& MaxStackSize == B.MaxStackSize;
	}

	FString ToString() const;
};

/**
 * When performing actions such as shooting, reloading, and so on,
 * the item's current state can be used to determine what actions are allowed or which ones should be triggered or deactivated.
 * 
 * For example, a totem that constantly regenerates health when being active.
 * ->	Can be set to inactive when the player is at full health or when the player is in combat. (or manually)
 */
UENUM()
enum class ECurrentItemState : uint8
{
	/** Don't consider the item has any state. */
	None = 0,

	/** The item is currently active. */
	Active = 1 << 0,

	/** The item is currently inactive. */
	Inactive = 1 << 1,

	/** The item is currently in the process of being activated. */
	Activating = 1 << 2,

	/** The item is currently in the process of being deactivated. */
	Deactivating = 1 << 3,
};

/**
 * Describes the active state of an item on a higher level.
 * Mostly used for user facing data.
 */
UENUM(BlueprintType)
enum class EActiveItemState : uint8
{
	/** Applies in all states, as long as the item is owned by the player. */
	Owned,

	/** Applies provided teh item is in an equippable inventory (needn't be equipped and active) */
	Equipped,

	/** Applies provided the item is both inside an equippable inventory and active (equipped) */
	EquippedAndActive,
};

/**
 * A message that is sent when the inventory changes.
 */
USTRUCT(BlueprintType)
struct FInventoryChangeMessage
{
	GENERATED_BODY()
	FInventoryChangeMessage() = default;
	FInventoryChangeMessage(UInventoryItemInstance* ItemThatChanged, const int32 OldCount, const int32 NewCount);
	
public:
	/** The actor that the inventory change occured on. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TWeakObjectPtr<AActor> Owner = nullptr;

	/** The controller that the inventory change occured on. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TWeakObjectPtr<AController> Controller = nullptr;

	/** The inventory manager that the inventory change occured on. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TWeakObjectPtr<UInventoryManager> InventoryManager = nullptr;

	/** The item that changed. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TObjectPtr<UInventoryItemInstance> Item = nullptr;

	/** The new stack count of the item. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	int32 NewStackCount = 0;

	/** The old stack count of the item. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	int32 OldStackCount = 0;

	/** The delta of the stack count. */
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	int32 Delta = 0;
};