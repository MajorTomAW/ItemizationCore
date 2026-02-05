// Author: Tom Werner (MajorT), 2025 November


#include "ViewModels/InventorySlotViewModel.h"

#include "Inventory/SlottableInventory.h"
#include "Items/IInventoryItemInstanceInterface.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InventorySlotViewModel)

UInventorySlotViewModel::UInventorySlotViewModel()
{
}

void UInventorySlotViewModel::SetInventoryAndSlot(ASlottableInventory* NewInventory, const FInventorySlotId& NewSlot, const FGameplayTag& NewGroupTag)
{
	if (OwningInventory != NewInventory || ItemSlotId != NewSlot)
	{
		if (IsValid(OwningInventory))
		{
			OwningInventory->OnItemAddedDelegate.RemoveAll(this);
			OwningInventory->OnItemRemovedDelegate.RemoveAll(this);
			OwningInventory->OnItemChangedDelegate.RemoveAll(this);
			OwningInventory->OnItemSlotChangedDelegate.RemoveAll(this);
		}

		OwningInventory = NewInventory;
		ItemSlotId = NewSlot;
		GroupTag = NewGroupTag;

		if (IsValid(OwningInventory))
		{
			OwningInventory->OnItemAddedDelegate.AddUObject(this, &ThisClass::OnItemAdded);
			OwningInventory->OnItemRemovedDelegate.AddUObject(this, &ThisClass::OnItemRemoved);
			OwningInventory->OnItemChangedDelegate.AddUObject(this, &ThisClass::OnItemChanged);
			OwningInventory->OnItemSlotChangedDelegate.AddUObject(this, &ThisClass::OnItemSlotChanged);
		}

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(OwningInventory);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ItemSlotId);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemStackSize);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemMaxStackSize);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSlotIdString);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSlotOccupied);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDefinition);

		ResolveItem();
	}
}


int32 UInventorySlotViewModel::GetItemStackSize() const
{
	if (ItemInstance)
	{
		if (const FInventoryItemEntry* ItemEntry = ItemInstance->GetItemEntry())
		{
			return ItemEntry->GetStackSize();
		}
	}

	return 0;
}

int32 UInventorySlotViewModel::GetItemMaxStackSize() const
{
	if (ItemInstance)
	{
		if (const FInventoryItemEntry* ItemEntry = ItemInstance->GetItemEntry())
		{
			return ItemEntry->GetItemDefinition()->GetMaxStackSize();
		}
	}

	return 0;
}

FString UInventorySlotViewModel::GetSlotIdString() const
{
	return ItemSlotId.ToString();
}

bool UInventorySlotViewModel::IsSlotOccupied() const
{
	return IsValid(ItemInstance.GetObject());
}

const UItemDefinitionBase* UInventorySlotViewModel::GetItemDefinition() const
{
	if (ItemInstance)
	{
		if (const auto* Entry = ItemInstance->GetItemEntry())
		{
			return Entry->GetItemDefinition();
		}
	}

	return nullptr;
}

void UInventorySlotViewModel::ResolveItem()
{
	UObject* NewItem = nullptr;
	if (IsValid(OwningInventory) && ItemSlotId.IsValid())
	{
		if (FInventoryItemSlot* Slot = OwningInventory->FindItemSlotBySlotId(ItemSlotId, GroupTag))
		{
			NewItem = Slot->GetItemInSlot();
		}
	}

	if (ItemInstance != NewItem)
	{
		ItemInstance = NewItem;

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ItemInstance);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSlotOccupied);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDefinition);
	}
}

void UInventorySlotViewModel::OnItemAdded(
	const FInventoryItemEntry& ItemThatWasAdded,
	const int32& LastCount,
	const int32& NewCount)
{
	ResolveItem();
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemStackSize);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemMaxStackSize);
}

void UInventorySlotViewModel::OnItemRemoved(
	const FInventoryItemEntry& ItemThatWasRemoved,
	const int32& LastCount,
	const int32& NewCount)
{
	ResolveItem();
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemStackSize);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemMaxStackSize);
}

void UInventorySlotViewModel::OnItemChanged(
	const FInventoryItemEntry& ItemThatChanged,
	const int32& LastCount,
	const int32& NewCount)
{
	ResolveItem();
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemStackSize);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemMaxStackSize);
}

void UInventorySlotViewModel::OnItemSlotChanged(const FInventoryItemSlot& ItemSlot)
{
	// Filter out for stranger slots
	if (ItemSlot.GetSlotId() != ItemSlotId)
	{
		return;
	}

	ResolveItem();
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemStackSize);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemMaxStackSize);
}
