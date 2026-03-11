// Author: Tom Werner (dc: majort), 2026

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVVMViewModelBase.h"
#include "InventoryGroupViewModel.generated.h"

#define UE_API ITEMIZATIONCOREUI_API

class IInventoryOwnerInterface;
struct FInventoryItemSlot;
struct FGameplayTag;
class AInventoryBase;
class UInventorySlotViewModel;

/** VM for managing the slots of a single inventory group. */
UCLASS(MinimalAPI)
class UInventoryGroupViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category=InventoryGroup, meta=(Categories="Inventory.Group"))
	static UInventoryGroupViewModel* CreateInventoryGroupViewModel(const AActor* InventoryOwner, FGameplayTag GroupTag);

	/** Assigns an inventory to this viewmodel. */
	UFUNCTION(BlueprintCallable, Category=InventoryGroup)
	UE_API virtual void SetInventoryAndGroup(AInventoryBase* NewInventory, const FGameplayTag& NewGroupTag);

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
	/** The owning inventory. */
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	TObjectPtr<AInventoryBase> OwningInventory;

	UPROPERTY()
	FGameplayTag GroupTag;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<TObjectPtr<UInventorySlotViewModel>> InventorySlotViewModels;
};

#undef UE_API
