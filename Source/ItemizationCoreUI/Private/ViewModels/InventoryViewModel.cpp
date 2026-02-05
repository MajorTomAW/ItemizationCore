// Author: Tom Werner (MajorT), 2025 November


#include "ViewModels/InventoryViewModel.h"

#include "MVVMGameSubsystem.h"
#include "Components/SlottableInventoryComponent.h"
#include "Inventory/SlottableInventory.h"
#include "ViewModels/InventoryGroupViewModel.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryViewModel)

TArray<UInventoryGroupViewModel*> UInventoryViewModel::GetInventoryGroupViewModels() const
{
	ThisClass* MutableThis = const_cast<ThisClass*>(this);

	const int32 OldNumGroups = InventoryGroupViewModels.Num();
	const TArray<FGameplayTag>& GroupTags = OwningInventory->GetAllItemSlotGroupTags();
	const int32 NumGroups = GroupTags.Num();

	if (OldNumGroups != NumGroups)
	{
		// Too many, need to delete
		if (OldNumGroups > NumGroups)
		{
			for (int32 Idx = OldNumGroups; Idx > NumGroups; Idx--)
			{
				TArray<FGameplayTag> Keys;
				InventoryGroupViewModels.GetKeys(Keys);

				MutableThis->InventoryGroupViewModels.Remove(Keys[Idx]);
			}
		}
		// Too few, need to add
		else
		{
			for (int32 Idx = OldNumGroups; Idx < NumGroups; Idx++)
			{
				FGameplayTag GroupTag = GroupTags[Idx];

				UInventoryGroupViewModel* GroupVM = NewObject<UInventoryGroupViewModel>(MutableThis);
				GroupVM->SetInventoryAndGroup(OwningInventory, GroupTag);

				MutableThis->InventoryGroupViewModels.Add(GroupTag, GroupVM);
			}
		}
	}

	TArray<TObjectPtr<UInventoryGroupViewModel>> ViewModels;
	InventoryGroupViewModels.GenerateValueArray(ViewModels);
	return ViewModels;
}

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
		// Either the actor is an inventory owner, or it has an component
		IInventoryOwnerInterface* InventoryOwner = Cast<IInventoryOwnerInterface>(OwningActor);
		if (InventoryOwner == nullptr)
		{
			TArray<UActorComponent*> Comps = OwningActor->GetComponentsByInterface(UInventoryOwnerInterface::StaticClass());
			if (!Comps.IsEmpty())
			{
				InventoryOwner = Cast<IInventoryOwnerInterface>(Comps[0]);
			}
		}

		if (InventoryOwner == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("CreateInventoryViewModel called on %s but it doesnt have an inventory"),
				*GetNameSafe(OwningActor))
			return nullptr;
		}

		ASlottableInventory* Inventory = Cast<ASlottableInventory>(InventoryOwner->GetInventory());
		if (!IsValid(Inventory))
		{
			UE_LOG(LogTemp, Error, TEXT("CreateInventoryViewModel called on %s but the inventory %s is not a slottable one"),
				*GetNameSafe(OwningActor), *GetNameSafe(InventoryOwner->GetInventory()))
			return nullptr;
		}

		ViewModel = NewObject<UInventoryViewModel>(OwningActor);
		ViewModel->SetInventory(Inventory);

		ViewModelCollection->AddViewModelInstance(Context, ViewModel);
	}

	check(IsValid(ViewModel))
	return ViewModel;
}

void UInventoryViewModel::SetInventory(ASlottableInventory* NewInventory)
{
	if (OwningInventory == NewInventory)
	{
		return;
	}

	if (IsValid(OwningInventory))
	{
		OwningInventory->OnItemAddedDelegate.RemoveAll(this);
		OwningInventory->OnItemRemovedDelegate.RemoveAll(this);
		OwningInventory->OnItemChangedDelegate.RemoveAll(this);
	}

	OwningInventory = NewInventory;

	if (IsValid(OwningInventory))
	{
		OwningInventory->OnItemAddedDelegate.AddUObject(this, &ThisClass::OnItemAdded);
		OwningInventory->OnItemRemovedDelegate.AddUObject(this, &ThisClass::OnItemRemoved);
		OwningInventory->OnItemChangedDelegate.AddUObject(this, &ThisClass::OnItemChanged);
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(OwningInventory);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetInventoryGroupViewModels);
}

void UInventoryViewModel::OnItemAdded(
	const FInventoryItemEntry& ItemThatWasAdded,
	const int32& LastCount,
	const int32& NewCount)
{
}

void UInventoryViewModel::OnItemRemoved(
	const FInventoryItemEntry& ItemThatWasRemoved,
	const int32& LastCount,
	const int32& NewCount)
{
}

void UInventoryViewModel::OnItemChanged(
	const FInventoryItemEntry& ItemThatChanged,
	const int32& LastCount,
	const int32& NewCount)
{
}
