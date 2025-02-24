// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryExtensionComponent.h"
#include "InventoryItemEntry.h"
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
class ITEMIZATIONCORERUNTIME_API UInventorySlotManager : public UInventoryExtensionComponent
{
	GENERATED_BODY()
	friend class UInventoryManager;
	friend class UEquipmentManager;

public:
	UInventorySlotManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Static getter to find the slot manager on an actor. */
	UFUNCTION(BlueprintPure, Category = ItemizationCore)
	static UInventorySlotManager* FindInventorySlotManager(AActor* Actor);

	//~ Begin UActorComponent Interface
	virtual void InitializeComponent() override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void GetReplicatedCustomConditionState(FCustomPropertyConditionState& OutActiveState) const override;
	//~ End UActorComponent Interface

	

	virtual void AddItemToSlot(const FInventoryItemEntry& ItemEntry);

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
};
