// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVVMViewModelBase.h"

#include "InventoryViewModel.generated.h"

struct FInventoryItemEntry;
class ASlottableInventory;
class UInventoryGroupViewModel;
/**
 *
 */
UCLASS()
class UInventoryViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, FieldNotify)
	TArray<UInventoryGroupViewModel*> GetInventoryGroupViewModels() const;

	UFUNCTION(BlueprintCallable)
	static UInventoryViewModel* CreateInventoryViewModel(AActor* OwningActor);

	UFUNCTION(BlueprintCallable)
	void SetInventory(ASlottableInventory* NewInventory);

protected:
	virtual void OnItemAdded(
		const FInventoryItemEntry& ItemThatWasAdded,
		const int32& LastCount,
		const int32& NewCount );

	virtual void OnItemRemoved(
		const FInventoryItemEntry& ItemThatWasRemoved,
		const int32& LastCount,
		const int32& NewCount);

	virtual void OnItemChanged(
		const FInventoryItemEntry& ItemThatChanged,
		const int32& LastCount,
		const int32& NewCount);

protected:
	/** The owning inventory of this vm. */
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	TObjectPtr<ASlottableInventory> OwningInventory;

	/** Cached list of view model for each inventory group. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UInventoryGroupViewModel>> InventoryGroupViewModels;
};
