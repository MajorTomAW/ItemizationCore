// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Items/InventoryItemSlot.h"
#include "InventorySlotViewModel.generated.h"

UCLASS()
class UInventorySlotViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UInventorySlotViewModel();

	UPROPERTY(BlueprintReadOnly, FieldNotify)
	FInventoryItemSlot ItemSlot;

public:
	UFUNCTION(BlueprintPure, FieldNotify)
	int32 GetItemCount() const;
};
