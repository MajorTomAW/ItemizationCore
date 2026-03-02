// Author: Tom Werner (dc: majort), 2026 January


#include "Components/SlottableInventoryComponent.h"

#include "ItemizationCoreLogChannels.h"
#include "Inventory/SlottableInventory.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif


USlottableInventoryComponent::USlottableInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USlottableInventoryComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (IsValid(InventoryConfig))
	{
		 TagContainer.AddTag(InventoryConfig->InventoryTag);
	}
}

ASlottableInventory* USlottableInventoryComponent::GetSlottableInventory() const
{
	return CastChecked<ASlottableInventory>(GetInventory());
}

FInventoryItemId USlottableInventoryComponent::PlaceItemInSlot_Definition(
	const UItemDefinitionBase* ItemDefinition,
	const FInventorySlotId& SlotId,
	int32 StackSize,
	UObject* SourceObject,
	FGameplayTag GroupTag,
	int32& OutNumCouldNotAdd)
{
	FInventoryItemId Result = FInventoryItemId::InvalidId;

	ASlottableInventory* Inventory = GetSlottableInventory();
	if (!IsValid(Inventory))
	{
		ITEMIZATION_ERROR_CONTEXT("Cannot place item [%s] in slot [%s] from inventory [%s] for %s. Inventory is invalid.",
			*GetNameSafe(ItemDefinition), *SlotId.ToString(), *GetNameSafe(Inventory), *GetNameSafe(GetOwner()));
		return Result;
	}

	// Build the place item in slot operation parameters
	FInventoryOp_PlaceItemInSlot::FParams Params;
	Params.ItemDefinition = ItemDefinition;
	Params.TargetSlotId = SlotId;
	Params.NumItems = StackSize;
	Params.GroupTag = GroupTag;
	Params.SourceObject = SourceObject;

	// Actually place the item into a slot
	if (const TInventoryOpPtr<FInventoryOp_PlaceItemInSlot> Op = Inventory->PlaceItemInSlot(MoveTemp(Params)))
	{
		Result = Op->Result.ItemId;
		OutNumCouldNotAdd = Op->Result.Excess;
	}

	return Result;
}



void USlottableInventoryComponent::SwapItemSlots(
	const FInventorySlotId& SlotA, FGameplayTag SlotGroupA,
	const FInventorySlotId& SlotB, FGameplayTag SlotGroupB)
{
	ASlottableInventory* Inventory = GetSlottableInventory();
	if (!IsValid(Inventory))
	{
		ITEMIZATION_ERROR_CONTEXT("Cannot swap SlotA [%s] with SlotB [%s] in inventory [%s] for %s. Inventory is invalid.",
			*SlotA.ToString(), *SlotB.ToString(), *GetNameSafe(Inventory), *GetNameSafe(GetOwner()));
		return;
	}

	// Build the swap item slot operation parameters
	FInventoryOp_SwapItemSlots::FParams Params;
	Params.SourceInventory = Inventory;
	Params.SourceSlotId = SlotA;
	Params.SourceGroupTag = SlotGroupA;
	Params.TargetInventory = Inventory;
	Params.TargetSlotId = SlotB;
	Params.TargetGroupTag = SlotGroupB;

	// Actually swap the item slots
	Inventory->SwapItemSlots(MoveTemp(Params));
}

#if WITH_EDITOR
EDataValidationResult USlottableInventoryComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!IsValid(InventoryClass) || !InventoryClass->IsChildOf(ASlottableInventory::StaticClass()))
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::FromString(FString::Printf(TEXT("Slottable Inventory component [%s] has an Inventory Class [%s] which is not type of ASlottableInventory"),
			*GetNameSafe(this), *GetNameSafe(InventoryClass))));
	}

	if (!IsValid(InventoryConfig))
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::FromString(FString::Printf(TEXT("Slottable Inventory component  [%s] has an invalid InventoryConfig asset."),
			*GetNameSafe(this))));
	}

	return Result;
}
#endif

void USlottableInventoryComponent::SetupInventory(AInventoryBase* Inventory)
{
	Super::SetupInventory(Inventory);

	if (HasAuthority())
	{
		InitInventoryGroups(Inventory);
	}
}

void USlottableInventoryComponent::InitInventoryGroups(AInventoryBase* Inventory)
{
	if (!IsValid(InventoryConfig))
	{
		ITEMIZATION_WARN("Unable to init inventory groups fpr %s with invalid inventory config.",
			*GetNameSafe(GetOwner()))
		return;
	}

	if (ASlottableInventory* SlottableInventory = GetSlottableInventory())
	{
		SlottableInventory->InitializeInventorySlots(InventoryConfig);
	}
}
