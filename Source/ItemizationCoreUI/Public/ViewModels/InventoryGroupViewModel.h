// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryGroupUIData.h"
#include "MVVMViewModelBase.h"
#include "InventoryGroupViewModel.generated.h"


class UInventorySlotViewModel;
/**
 *
 */
UCLASS()
class UInventoryGroupViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	FInventoryGroupUIData GroupUIData;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<TObjectPtr<UInventorySlotViewModel>> InventorySlots;
};
