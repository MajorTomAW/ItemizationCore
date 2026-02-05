// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Items/InventoryItemSlot.h"
#include "InventorySlotViewModel.generated.h"

class UItemDefinitionBase;
class IInventoryItemInstanceInterface;

#define UE_API ITEMIZATIONCOREUI_API

UCLASS(MinimalAPI)
class UInventorySlotViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UE_API UInventorySlotViewModel();

	UFUNCTION(BlueprintCallable)
	UE_API void SetInventoryAndSlot(ASlottableInventory* NewInventory, const FInventorySlotId& NewSlot, const FGameplayTag& NewGroupTag);

	UFUNCTION(BlueprintPure, FieldNotify)
	UE_API int32 GetItemCount() const;

	UFUNCTION(BlueprintPure, FieldNotify)
	UE_API FString GetSlotIdString() const;

	UFUNCTION(BlueprintPure, FieldNotify)
	UE_API bool IsSlotOccupied() const;

	UFUNCTION(BlueprintPure, FieldNotify)
	UE_API const UItemDefinitionBase* GetItemDefinition() const;

	/** Returns the slot id of this slot vm. */
	FInventorySlotId GetItemSlotId() const { return ItemSlotId; }

	/** Returns the owning inventory of this slot vm. */
	ASlottableInventory* GetOwningInventory() const { return OwningInventory; }

	/** Returns the owning item instance of this slot vm. */
	TScriptInterface<IInventoryItemInstanceInterface> GetItemInstance() const { return ItemInstance; }

protected:
	UE_API void ResolveItem();

	UE_API virtual void OnItemAdded(
		const FInventoryItemEntry& ItemThatWasAdded,
		const int32& LastCount,
		const int32& NewCount );

	UE_API virtual void OnItemRemoved(
		const FInventoryItemEntry& ItemThatWasRemoved,
		const int32& LastCount,
		const int32& NewCount);

	UE_API virtual void OnItemChanged(
		const FInventoryItemEntry& ItemThatChanged,
		const int32& LastCount,
		const int32& NewCount);

	UE_API virtual void OnItemSlotChanged(const FInventoryItemSlot& ItemSlot);

protected:
	UPROPERTY(BlueprintReadOnly, Getter, FieldNotify)
	TObjectPtr<ASlottableInventory> OwningInventory;

	UPROPERTY(BlueprintReadOnly, Getter, FieldNotify)
	FInventorySlotId ItemSlotId;
	FGameplayTag GroupTag;

	UPROPERTY(BlueprintReadOnly, Getter, FieldNotify)
	TScriptInterface<IInventoryItemInstanceInterface> ItemInstance;
};

#undef UE_API
