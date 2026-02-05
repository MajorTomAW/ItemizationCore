// Author: Tom Werner (MajorT), 2025 November


#include "ViewModels/InventoryGroupViewModel.h"

#include "Inventory/SlottableInventory.h"
#include "ViewModels/InventorySlotViewModel.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryGroupViewModel)

void UInventoryGroupViewModel::SetInventoryAndGroup(ASlottableInventory* NewInventory, const FGameplayTag& NewGroupTag)
{
	GroupTag = NewGroupTag;

	if (NewInventory == OwningInventory)
	{
		return;
	}

	if (IsValid(OwningInventory))
	{

	}

	OwningInventory = NewInventory;

	if (IsValid(OwningInventory))
	{

	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNumSlotsInGroup);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetInventorySlotViewModels);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNumColumns);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNumRows);
}

TArray<UInventorySlotViewModel*> UInventoryGroupViewModel::GetInventorySlotViewModels() const
{
	UInventoryGroupViewModel* MutableThis = const_cast<ThisClass*>(this);

	const int32 OldNumSlots = InventorySlotViewModels.Num();
	const TArray<const FInventoryItemSlot*>& Slots = GetItemSlotsInGroup();
	const int32 NumSlots = Slots.Num();

	if (OldNumSlots != NumSlots)
	{
		MutableThis->InventorySlotViewModels.SetNum(NumSlots);

		// Create new view model
		for (int32 SlotIdx = OldNumSlots; SlotIdx < NumSlots; SlotIdx++)
		{
			const FInventoryItemSlot* Slot = Slots[SlotIdx];
			MutableThis->InventorySlotViewModels[SlotIdx] = MutableThis->CreateInventorySlotViewModel(*Slot);
		}
	}

	return InventorySlotViewModels;
}

int32 UInventoryGroupViewModel::GetNumSlotsInGroup() const
{
	return IsValid(OwningInventory) ? OwningInventory->GetItemSlotsInGroup(GroupTag).Num() : 0;
}

TArray<const FInventoryItemSlot*> UInventoryGroupViewModel::GetItemSlotsInGroup() const
{
	return IsValid(OwningInventory) ? OwningInventory->GetItemSlotsInGroup(GroupTag) : TArray<const FInventoryItemSlot*>();
}

int32 UInventoryGroupViewModel::GetNumRows() const
{
	return IsValid(OwningInventory) ? OwningInventory->GetNumRowsInGroup(GroupTag) : 0;
}

int32 UInventoryGroupViewModel::GetNumColumns() const
{
	return IsValid(OwningInventory) ? OwningInventory->GetNumColumnsInGroup(GroupTag) : 0;
}

UInventorySlotViewModel* UInventoryGroupViewModel::CreateInventorySlotViewModel(const FInventoryItemSlot& Slot)
{
	UInventorySlotViewModel* SlotVM = NewObject<UInventorySlotViewModel>(this);
	SlotVM->SetInventoryAndSlot(OwningInventory, Slot.GetSlotId(), GroupTag);
	return SlotVM;
}
