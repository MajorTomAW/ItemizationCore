// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryGroupUIData.h"
#include "MVVMViewModelBase.h"
#include "Items/InventoryItemSlot.h"
#include "InventoryGroupViewModel.generated.h"


struct FInventoryItemSlot;
class ASlottableInventory;
class UInventorySlotViewModel;

#define UE_API ITEMIZATIONCOREUI_API
/**
 *
 */
UCLASS(MinimalAPI)
class UInventoryGroupViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** Assigns an inventory to this viewmodel. */
	UFUNCTION(BlueprintCallable, Category=InventoryGroup)
	UE_API virtual void SetInventoryAndGroup(ASlottableInventory* NewInventory, const FGameplayTag& NewGroupTag);

	/** Returns all slot view models. */
	UFUNCTION(BlueprintPure, FieldNotify)
	UE_API TArray<UInventorySlotViewModel*> GetInventorySlotViewModels() const;

	/** Returns the number of slots in this group. */
	UFUNCTION(BlueprintPure, FieldNotify)
	UE_API int32 GetNumSlotsInGroup() const;
	UE_API TArray<const FInventoryItemSlot*> GetItemSlotsInGroup() const;

	UFUNCTION(BlueprintPure, FieldNotify)
	UE_API int32 GetNumRows() const;

	UFUNCTION(BlueprintPure, FieldNotify)
	UE_API int32 GetNumColumns() const;

protected:
	UE_API virtual UInventorySlotViewModel* CreateInventorySlotViewModel(const FInventoryItemSlot& Slot);

public:
	/** The UI data about this inventory group. */
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	FInventoryGroupUIData GroupUIData;

	/** The owning inventory. */
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	TObjectPtr<ASlottableInventory> OwningInventory;
	FGameplayTag GroupTag;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<TObjectPtr<UInventorySlotViewModel>> InventorySlotViewModels;
};

#undef UE_API
