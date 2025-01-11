// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Controller.h"

#include "ItemizationCoreTypes.generated.h"

class UInventoryManager;
struct FInventoryItemEntry;
class UItemDefinition;
class UInventoryItemInstance;
class AController;
class AActor;

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
public:
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	FItemActionContextData(FItemActionContextData&&) = default;
	FItemActionContextData(const FItemActionContextData&) = default;
	FItemActionContextData& operator=(FItemActionContextData&&) = default;
	FItemActionContextData& operator=(const FItemActionContextData&) = default;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
	FItemActionContextData()
		: Instigator(nullptr)
		, InventoryManager(nullptr)
		, ContextTags(nullptr)
		, Delta(0)
		, SlotSize(1)
		, MaxStackSize(1)
	{
	}
	
	FItemActionContextData(const FInventoryItemEntry& ItemEntry);
	
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
 * Describes the active state of an item on a higher level.
 * Mostly used for user-facing data or logic that should only be applied in certain states.
 */
UENUM(BlueprintType)
enum class EUserFacingItemState : uint8
{
	/** Applies in all states, as long as the item is owned by the player. */
	Owned = 0,

	/** Applies provided the item is in an equippable inventory (needn't be equipped and active) */
	Equipped = 1 << 1,

	/** Applies provided the item is both inside an equippable inventory and active (equipped) */
	EquippedAndActive = 1 << 2,
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

/**
 * Utility-struct that holds some core data about the inventory system.
 * Usually associated with the actor that owns the inventory.
 *
 * Has the concept of owning and avatar actor to make it possible to split up the inventory system from the actual actor.
 * As some games might want the inventory to be on the player controller,
 * while the actual physical representation of the items is on the player character.
 */
USTRUCT(BlueprintType)
struct FItemizationCoreInventoryData
{
	GENERATED_USTRUCT_BODY()

public:
	FItemizationCoreInventoryData();
	virtual ~FItemizationCoreInventoryData() = default;

public:
	/** The owning actor that owns the inventory. (virtually) !!Should always be valid!! */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Data")
	TWeakObjectPtr<AActor> OwnerActor;

	/** The avatar actor that represents the owner actor. (physically) !!Should always be valid!!*/
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Data")
	TWeakObjectPtr<AActor> AvatarActor;

	/** The controller associated with the owning actor. !!Could be invalid!!*/
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Data")
	TWeakObjectPtr<AController> Controller;

	/** The inventory manager that is associated with the owning actor. !!Should always be valid!!*/
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Data")
	TWeakObjectPtr<UInventoryManager> InventoryManager;

public:
	/**
	 * Checks if the actor is locally controlled.
	 * Only true for players on the client that owns them.
	 * (Unlike APawn::IsLocallyControlled() which requires a controller)
	 * @returns True if the actor is locally controlled.
	 */
	bool IsLocallyControlled() const;

	/**
	 * Checks if the actual player controller is locally controlled.
	 * @returns True, if the actor has a player controller that is locally controlled. 
	 */
	bool IsLocallyControlledPlayer() const;

	/**
     * Checks if the owning actor has net authority.
     * @returns True if the owning actor has net authority.
     */
	bool HasNetAuthority() const;

	/**
     * Initializes the actor info from an owning actor.
     * Will set both owner and avatar.
     * @param InOwnerActor The actor that owns the abilities.
     * @param InAvatarActor The physical representation of the owner.
     * @param InInventoryManager The inventory manager that is associated with the owning actor.
     */
	virtual void InitFromActor(AActor* InOwnerActor, AActor* InAvatarActor, UInventoryManager* InInventoryManager);

	/**
     * Sets a new avatar actor, keeping the same owner and ability system component.
     * @param NewAvatarActor The new avatar actor.
     */
	virtual void SetAvatarActor_Direct(AActor* NewAvatarActor);

	/**
     * Clears out any actor info, both owner and avatar.
     */
	virtual void ClearInventoryData();

	/** Converts the inventory data to a string. */
	FString ToString() const;
};

/**
 * Helper struct to cache off all granted ability-, effect handles, and attributes.
 * That were given by an item or equipment.
 */
USTRUCT()
struct FItemizationGrantedHandles
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilityHandles;

	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> EffectHandles;

	UPROPERTY()
	TArray<TObjectPtr<const class UAttributeSet>> AttributeSets;
};
