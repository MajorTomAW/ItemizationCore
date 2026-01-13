// Author: Tom Werner (MajorT), 2025 November


#include "ViewModels/InventoryViewModel.h"

#include "MVVMGameSubsystem.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryViewModel)

UInventoryViewModel* UInventoryViewModel::CreateInventoryViewModel(AActor* OwningActor)
{
	if (!ensure(IsValid(OwningActor)))
	{
		return nullptr;
	}

	UGameInstance* GameInstance = OwningActor->GetWorld() ? OwningActor->GetWorld()->GetGameInstance() : nullptr;
	if (!IsValid(GameInstance))
	{
		return nullptr;
	}

	// Get the VM subsystem
	UMVVMGameSubsystem* ViewModelSub = GameInstance->GetSubsystem<UMVVMGameSubsystem>();
	check(IsValid(ViewModelSub))

	// Get the VM collection
	UMVVMViewModelCollectionObject* ViewModelCollection = ViewModelSub->GetViewModelCollection();
	check(IsValid(ViewModelCollection))

	// Try to find an existing viewmodel
	FMVVMViewModelContext Context;
	Context.ContextClass = StaticClass();
	Context.ContextName = *FString::Printf(TEXT("%s_%s"), *OwningActor->GetName(), *StaticClass()->GetName());

	UInventoryViewModel* ViewModel = Cast<UInventoryViewModel>(ViewModelCollection->FindViewModelInstance(Context));

	// No viewmodel found, create a new one!
	if (!IsValid(ViewModel))
	{
		ViewModel = NewObject<UInventoryViewModel>(OwningActor);
		ViewModelCollection->AddViewModelInstance(Context, ViewModel);
	}

	check(IsValid(ViewModel))
	return ViewModel;
}
