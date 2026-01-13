// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVVMViewModelBase.h"

#include "InventoryViewModel.generated.h"

class UInventoryGroupViewModel;
/**
 *
 */
UCLASS()
class UInventoryViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TMap<FGameplayTag, TObjectPtr<UInventoryGroupViewModel>> InventoryGroups;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UInventoryViewModel* CreateInventoryViewModel(AActor* OwningActor);
};
