// Author: Tom Werner (dc: majort), 2026

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventorySlotId.h"
#include "MVVMViewModelBase.h"
#include "InventorySlotViewModel.generated.h"

struct FInventoryItemEntry;
struct FInventoryItemSlot;
struct FInventoryItemId;
class UItemInstanceBase;
class AInventoryBase;

/** A VM representing a single slot in an inventory "grid".  */
UCLASS()
class ITEMIZATIONCOREUI_API UInventorySlotViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void SetInventoryAndSlot(AInventoryBase* NewInventory, const FInventorySlotId& NewSlotId, const FGameplayTag& NewGroupTag);

	/** Gets the current stack size of the item in this slot. */
	UFUNCTION(BlueprintPure, FieldNotify)
	int32 GetItemStackSize() const;

	/** Gets the current max stack size of the item in this slot. */
	UFUNCTION(BlueprintPure, FieldNotify)
	int32 GetItemMaxStackSize() const;

	/** Gets the current item definition of the item in this slot. */
	UFUNCTION(BlueprintPure, FieldNotify)
	const class UItemDefinitionBase* GetItemDefinition() const;

	/** Returns the slot id of this slot vm. */
	UFUNCTION(BlueprintPure)
	FInventorySlotId GetItemSlotId() const { return ItemSlotId; }

	/** returns the item instance of this slot vm. */
	UFUNCTION(BlueprintPure)
	UItemInstanceBase* GetItemInstance() const { return ItemInstance; }

	/** Returns true if this slot contains a valid item. */
	UFUNCTION(BlueprintPure, FieldNotify)
	bool IsSlotOccupied() const;

	UFUNCTION(BlueprintPure)
	FGameplayTag GetGroupTag() const { return GroupTag; }

	UFUNCTION(BlueprintPure)
	AInventoryBase* GetOwningInventory() const { return OwningInventory; }

protected:
	void TryResolveItem();

	void BroadcastNewItem();
	void BroadcastItemStateChanged();

	virtual void OnItemSlotChanged(const FInventoryItemSlot& SlotThatChanged, const FInventoryItemId& LastItemInSlot, const FInventoryItemId& NewItemInSlot);
	virtual void OnItemChanged(const FInventoryItemEntry& ItemThatChanged, const int32& LastStackSize, const int32& NewStackSize);

protected:
	/** The owning inventory. */
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	TObjectPtr<AInventoryBase> OwningInventory;

	UPROPERTY(BlueprintReadOnly, Getter, FieldNotify)
	FInventorySlotId ItemSlotId;

	UPROPERTY()
	FGameplayTag GroupTag;

	UPROPERTY(BlueprintReadOnly, Getter, FieldNotify)
	TObjectPtr<UItemInstanceBase> ItemInstance;
};
