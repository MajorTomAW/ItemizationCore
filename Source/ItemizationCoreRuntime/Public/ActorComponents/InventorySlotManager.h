// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventorySlotEntry.h"
#include "Components/ActorComponent.h"
#include "InventorySlotManager.generated.h"

struct FItemizationCoreInventoryData;
/**
 * Manages the slots of an inventory.
 * This component is meant to be used in conjunction with the InventoryManager component. 
 * Slots are purely virtual and don't have any physical representation.
 * Also, they don't store any items,
 * rather they contain the cached-off handle to the item which lives in the inventory.
 *
 * Slots are primarily used to manage and organize items in the inventory grid.
 * By default, slots aren't limited to any specific item type and have to be configured via the item traits.
 *
 * Having slots fits for many different inventory systems, for example, a grid-based inventory system.
 *
 * @see UInventoryManager
 */
UCLASS(ClassGroup=(Itemization), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class ITEMIZATIONCORERUNTIME_API UInventorySlotManager : public UActorComponent
{
	GENERATED_BODY()
	friend class UInventoryManager;
	friend class UEquipmentManager;

public:
	UInventorySlotManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Static getter to find the slot manager on an actor. */
	static UInventorySlotManager* GetSlotManager(AActor* Actor);

	/** Called to link up this equipment manager with an inventory manager. */
	virtual void SetInventoryManager(UInventoryManager* InInventoryManager);

	/** Called to initialize the equipment manager with the inventory manager. */
	virtual void TryInitializeWithInventoryManager();

	/** Returns the inventory data. */
	const TSharedPtr<FItemizationCoreInventoryData>& GetInventoryData() const;
	FItemizationCoreInventoryData* GetInventoryDataPtr() const;

	/**
	 * Retrieves the Inventory Manager that owns the inventory.
	 * Will first try to resolve the weak reference, if that fails from the inventory data and lastly from the owning actor.
	 */
	UInventoryManager* GetInventoryManager() const;

	/** Gets the pawn that owns the component, this should always be valid during gameplay but can return null in the editor */
	template <class T>
	T* GetPawn() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "'T' template parameter to 'GetPawn' must be a subclass of APawn");
		return Cast<T>(GetPawn());
	}

	/** Gets the pawn that owns the component, this should always be valid during gameplay but can return null in the editor */
	APawn* GetPawn() const;

	/** Gets the controller that owns the component, this should always be valid during gameplay but can return null in the editor */
	template <class T>
	T* GetController() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AController>::Value, "'T' template parameter to 'GetController' must be a subclass of AController");
		return Cast<T>(GetController());
	}

	/** Gets the controller that owns the component, this should always be valid during gameplay but can return null in the editor */
	AController* GetController() const;

	/** Returns true if this component's actor has authority. */
	virtual bool IsOwnerActorAuthoritative() const;

	//~ Begin UObject Interface
	virtual void PreNetReceive() override;
	virtual void BeginPlay() override;
	//~ End UObject Interface

	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const override;
	//~ End UActorComponent Interface

	/**
	 * Returns a list with all slot entries.
	 * @param OutSlotEntries The list to fill with the slot entries.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Itemization Core")
	void GetAllSlotEntries(TArray<FInventorySlotEntry>& OutSlotEntries) const;

protected:
	/** The number of default slots that will exist in the inventory. */
	UPROPERTY(EditAnywhere, Category = Slots, meta = (ClampMin=1, UIMin=1))
	int32 DefaultSlotCount = 20;

	/** The replicated slot entry list. */
	UPROPERTY(Replicated, BlueprintReadOnly, Transient, Category = Slots)
	FInventorySlotContainer SlotEntries;

private:
	/**
	 * Cached data about the equipment system such as the inventory manager, etc.
	 * Utility-struct for easy access to those data.
	 *
	 * For simulated proxies this will be its own inventory data not linked to the inventory manager.
	 */
	TSharedPtr<FItemizationCoreInventoryData> CachedInventoryData;
};
