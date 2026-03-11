// Author: Tom Werner (dc: majort), 2026 January


#include "InventoryLibrary.h"

#include "IInventoryOwnerInterface.h"
#include "InventoryBase.h"
#include "Items/Data/ItemComponentData_MaxStackSize.h"

FInventorySlotId UInventoryLibrary::MakeInventorySlotId(int64 Row, int64 Column)
{
	// MJT I had to expose this as int64 to get the full range of uint32
	return FInventorySlotId(static_cast<uint32>(Row), static_cast<uint32>(Column));
}

void UInventoryLibrary::BreakInventorySlotId(const FInventorySlotId& SlotId, int64& Row, int64& Column)
{
	Row = static_cast<int64>(SlotId.GetRowIndex());
	Column = static_cast<int64>(SlotId.GetColumnIndex());
}

FInventoryItemId UInventoryLibrary::MakeInventoryItemId(int64 Id)
{
	// MJT I had to expose this as int64 to get the full range of uint32
	return FInventoryItemId(static_cast<uint32>(Id));
}

void UInventoryLibrary::BreakInventoryItemId(const FInventoryItemId& ItemId, int64& Id)
{
	Id = static_cast<int64>(ItemId.Get());
}

AInventoryBase* UInventoryLibrary::FindInventory(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	if (Actor->Implements<UInventoryOwnerInterface>())
	{
		return IInventoryOwnerInterface::Execute_GetInventory(Actor);
	}

	if (UActorComponent* AC = Actor->FindComponentByInterface(UInventoryOwnerInterface::StaticClass()))
	{
		return IInventoryOwnerInterface::Execute_GetInventory(AC);
	}

	return nullptr;
}

UItemInstanceBase* UInventoryLibrary::GetItem(const AInventoryBase* Inventory, const FInventoryItemId& ItemId)
{
	return Inventory->GetItem(ItemId);
}

UItemInstanceBase* UInventoryLibrary::GetItemInSlot(
	const AInventoryBase* Inventory,
	const FInventorySlotId& SlotId,
	FGameplayTag GroupTag)
{
	return Inventory->GetItemInSlot(SlotId, GroupTag);
}

bool UInventoryLibrary::ItemHasTrait(const UItemDefinitionBase* ItemDefinition, FGameplayTag TraitToCheck)
{
	return IsValid(ItemDefinition) && ItemDefinition->HasTrait(TraitToCheck);
}

bool UInventoryLibrary::IsItemStackable(const UItemDefinitionBase* ItemDefinition)
{
	if (!IsValid(ItemDefinition))
	{
		return false;
	}

	if (const auto* MaxStackData = ItemDefinition->GetItemData<FItemComponentData_MaxStackSize>())
	{
		return MaxStackData->GetMaxStackSize() > 1;
	}

	return false;
}

FInventorySlotId UInventoryLibrary::FindSlotId(
	AInventoryBase* Inventory,
	const UItemInstanceBase* ItemInstance,
	FGameplayTag& OutGroupTag)
{
	FInventorySlotId SlotId = FInventorySlotId::InvalidId;
	if (!IsValid(Inventory))
	{
		return SlotId;
	}

	if (const auto* Slot = Inventory->FindItemSlotByItemId(ItemInstance->GetItemId(), FGameplayTag()))
	{
		OutGroupTag = Slot->GetGroupTag();
		SlotId = Slot->GetSlotId();
	}

	return SlotId;
}

UItemInstanceBase* UInventoryLibrary::GiveItemToInventoryOwner(
	const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner,
	const UItemDefinitionBase* ItemDefinition,
	int32 NumToGive,
	UObject* SourceObject,
	FGameplayTag PreferredGroup,
	int32& OutNumCouldNotGive)
{
	AInventoryBase* Inventory = InventoryOwner->Execute_GetInventory(InventoryOwner.GetObject());
	if (!ensure(IsValid(Inventory)))
	{
		return nullptr;
	}

	if (Inventory->HasAuthority())
	{
		return Inventory->GiveItem(ItemDefinition, NumToGive, SourceObject, PreferredGroup, OutNumCouldNotGive);
	}
	else
	{
		//@TODO: Do some server rpc idk
	}

	return nullptr;
}

void UInventoryLibrary::Server_GiveItemToInventoryOwner(
	const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner,
	const UItemDefinitionBase* ItemDefinition,
	int32 NumToGive,
	UObject* SourceObject,
	FGameplayTag PreferredGroup)
{
	AInventoryBase* Inventory = InventoryOwner->Execute_GetInventory(InventoryOwner.GetObject());
	if (!ensure(IsValid(Inventory)))
	{
		return;
	}

	if (Inventory->HasAuthority())
	{
		int32 OutNumCouldNotGive;
		Inventory->GiveItem(ItemDefinition, NumToGive, SourceObject, PreferredGroup, OutNumCouldNotGive);
	}
	else
	{
		Inventory->Server_GiveItem(ItemDefinition, NumToGive, SourceObject, PreferredGroup);
	}
}

UItemInstanceBase* UInventoryLibrary::PlaceItemInSlotForInventoryOwner(
	const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner,
	const UItemDefinitionBase* ItemDefinition,
	int32 NumToGive,
	UObject* SourceObject,
	const FInventorySlotId& SlotId,
	FGameplayTag GroupTag,
	int32& OutNumCouldNotGive)
{
	AInventoryBase* Inventory = InventoryOwner->Execute_GetInventory(InventoryOwner.GetObject());
	if (!ensure(IsValid(Inventory)))
	{
		return nullptr;
	}

	if (Inventory->HasAuthority())
	{
		return Inventory->PlaceItemInSlot(ItemDefinition, NumToGive, SourceObject, SlotId, GroupTag, OutNumCouldNotGive);
	}

	return nullptr;
}

int32 UInventoryLibrary::RemoveItemFromInventoryOwner(
	const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner,
	const UItemInstanceBase* Item,
	int32 NumToRemove)
{
	AInventoryBase* Inventory = InventoryOwner->Execute_GetInventory(InventoryOwner.GetObject());
	if (!ensure(IsValid(Inventory)))
	{
		return NumToRemove;
	}

	if (!Inventory->HasAuthority())
	{
		return NumToRemove;
	}

	return Inventory->RemoveItem(Item, NumToRemove);
}

void UInventoryLibrary::SwapItemSlotsOnInventoryOwner(
	const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner,
	const FInventorySlotId& SlotA, FGameplayTag GroupTagA,
	const FInventorySlotId& SlotB, FGameplayTag GroupTagB)
{
	AInventoryBase* Inventory = InventoryOwner->Execute_GetInventory(InventoryOwner.GetObject());
	if (!ensure(IsValid(Inventory)))
	{
		return;
	}

	if (!Inventory->HasAuthority())
	{
		return;
	}

	Inventory->SwapItemSlots(SlotA, GroupTagA, SlotB, GroupTagB);
}

void UInventoryLibrary::Server_SwapItemSlotsOnInventoryOwner(
	const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner,
	const FInventorySlotId& SlotA, FGameplayTag GroupTagA,
	const FInventorySlotId& SlotB, FGameplayTag GroupTagB)
{
	AInventoryBase* Inventory = InventoryOwner->Execute_GetInventory(InventoryOwner.GetObject());
	if (!ensure(IsValid(Inventory)))
	{
		return;
	}

	if (Inventory->HasAuthority())
	{
		Inventory->SwapItemSlots(SlotA, GroupTagA, SlotB, GroupTagB);
	}
	else
	{
		Inventory->Server_SwapItemSlots(SlotA, GroupTagA, SlotB, GroupTagB);
	}
}

void UInventoryLibrary::Server_DropItem(
	const TScriptInterface<IInventoryOwnerInterface>& InventoryOwner,
	const FInventoryItemId& ItemId, int32 NumToDrop)
{
	AInventoryBase* Inventory = InventoryOwner->Execute_GetInventory(InventoryOwner.GetObject());
	if (!ensure(IsValid(Inventory)))
	{
		return;
	}

	if (Inventory->HasAuthority())
	{
		Inventory->DropItem(ItemId, NumToDrop);
	}
	else
	{
		Inventory->Server_DropItem(ItemId, NumToDrop);
	}
}
