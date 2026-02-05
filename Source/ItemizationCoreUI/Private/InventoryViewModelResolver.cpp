// Copyright © 2025 Playton. All Rights Reserved.


#include "InventoryViewModelResolver.h"

#include "Blueprint/UserWidget.h"
#include "ViewModels/InventoryViewModel.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryViewModelResolver)

UObject* UInventoryViewModelResolver::CreateInstance(
	const UClass* ExpectedType,
	const UUserWidget* UserWidget,
	const UMVVMView* View) const
{
	return UInventoryViewModel::CreateInventoryViewModel(UserWidget->GetOwningPlayer());
}
