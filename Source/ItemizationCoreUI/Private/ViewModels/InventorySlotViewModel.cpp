// Author: Tom Werner (dc: majort), 2026


#include "ViewModels/InventorySlotViewModel.h"

#include "InventoryBase.h"
#include "ItemizationCoreLogChannels.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InventorySlotViewModel)

void UInventorySlotViewModel::SetInventoryAndSlot(
	AInventoryBase* NewInventory,
	const FInventorySlotId& NewSlotId,
	const FGameplayTag& NewGroupTag)
{
	if (OwningInventory != NewInventory ||
		ItemSlotId != NewSlotId ||
		GroupTag != NewGroupTag)
	{
		if (IsValid(OwningInventory))
		{
			OwningInventory->OnItemSlotChangedDelegate.RemoveAll(this);
			OwningInventory->OnItemChangedDelegate.RemoveAll(this);
		}

		OwningInventory = NewInventory;
		ItemSlotId = NewSlotId;
		GroupTag = NewGroupTag;

		if (IsValid(OwningInventory))
		{
			// Slot changed: To read new items that were placed in the slot
			OwningInventory->OnItemSlotChangedDelegate.AddUObject(this, &ThisClass::OnItemSlotChanged);

			// Item changed: To read new item state, stack count, etc.
			OwningInventory->OnItemChangedDelegate.AddUObject(this, &ThisClass::OnItemChanged);
		}

		TryResolveItem();

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(OwningInventory);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ItemSlotId);
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
		return ItemInstance->GetItemDefinition()->GetMaxStackSize();
	}

	return 0;
}

const class UItemDefinitionBase* UInventorySlotViewModel::GetItemDefinition() const
{
	if (ItemInstance)
	{
		return ItemInstance->GetItemDefinition();
	}

	return nullptr;
}

bool UInventorySlotViewModel::IsSlotOccupied() const
{
	return IsValid(ItemInstance);
}

void UInventorySlotViewModel::TryResolveItem()
{
	UItemInstanceBase* NewItem = nullptr;
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
		BroadcastNewItem();
	}
}

void UInventorySlotViewModel::BroadcastNewItem()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ItemInstance);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSlotOccupied);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemDefinition);

	ITEMIZATION_UI_LOG(Display, "BroadcastNewItem %s", *ItemSlotId.ToString())

	BroadcastItemStateChanged();
}

void UInventorySlotViewModel::BroadcastItemStateChanged()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemStackSize);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemMaxStackSize);
}

void UInventorySlotViewModel::OnItemSlotChanged(
	const FInventoryItemSlot& SlotThatChanged,
	const FInventoryItemId& LastItemInSlot,
	const FInventoryItemId& NewItemInSlot)
{
	if (SlotThatChanged.GetSlotId() != ItemSlotId)
	{
		return;
	}

	TryResolveItem();
}

void UInventorySlotViewModel::OnItemChanged(
	const FInventoryItemEntry& ItemThatChanged,
	const int32& LastStackSize,
	const int32& NewStackSize)
{
	if (ItemInstance == ItemThatChanged.GetItemInstance())
	{
		BroadcastItemStateChanged();
	}
}
