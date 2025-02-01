// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationCoreStatics.h"

#include "InventoryItemEntry.h"
#include "ItemizationCoreLog.h"
#include "ItemizationCoreTypes.h"
#include "ActorComponents/InventoryManager.h"
#include "Components/ItemComponentData_Icon.h"

FString UItemizationCoreStatics::Conv_ItemHandleToString(const FInventoryItemEntryHandle& Handle)
{
	return Handle.ToString();
}

bool UItemizationCoreStatics::EqualEqual_ItemHandle(const FInventoryItemEntryHandle& A, const FInventoryItemEntryHandle& B)
{
	return A == B;
}

bool UItemizationCoreStatics::NotEqual_ItemHandle(const FInventoryItemEntryHandle& A, const FInventoryItemEntryHandle& B)
{
	return A != B;
}

const UInventoryItemInstance* UItemizationCoreStatics::GetItemInstanceFromHandle(UInventoryManager* InventoryManager, const FInventoryItemEntryHandle& ItemHandle)
{
	// Validate the inventory manager.
	if (InventoryManager == nullptr)
	{
		ITEMIZATION_LOG(Error, TEXT("GetItemInstanceFromHandle() called with an invalid inventory manager."));
		return nullptr;
	}

	// Get and validate the item entry
	const FInventoryItemEntry* ItemEntry = InventoryManager->FindItemEntryFromHandle(ItemHandle);
	if (ItemEntry == nullptr)
	{
		ITEMIZATION_LOG(Error, TEXT("GetItemInstanceFromHandle() Item entry not found for handle %s."), *ItemHandle.ToString());
		return nullptr;
	}

	const UInventoryItemInstance* Instance = ItemEntry->Instance;
	if (Instance == nullptr)
	{
		ITEMIZATION_LOG(Error, TEXT("GetItemInstanceFromHandle() Item instance not found for handle %s."), *ItemHandle.ToString());
		return nullptr;
	}

	return Instance;
}

TSoftObjectPtr<UTexture2D> UItemizationCoreStatics::GetItemIcon(const UItemDefinition* ItemDefinition)
{
	if (const FItemComponentData_Icon* IconComp = ItemDefinition->GetDisplayComponent<FItemComponentData_Icon>())
	{
		return IconComp->Icon;
	}

	return nullptr;
}

void UItemizationCoreStatics::GetAbilityComponentData(
	const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_Ability& OutData)
{
	if (ItemDefinition == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	const FItemComponentData_Ability* AbilityComp = ItemDefinition->GetItemComponent<FItemComponentData_Ability>();
	if (AbilityComp == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	OutData = *AbilityComp;
	bOutSuccess = true;
}

void UItemizationCoreStatics::GetEquipmentComponentData(
	const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_Equipment& OutData)
{
	if (ItemDefinition == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	const FItemComponentData_Equipment* EquipmentComp = ItemDefinition->GetItemComponent<FItemComponentData_Equipment>();
	if (EquipmentComp == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	OutData = *EquipmentComp;
	bOutSuccess = true;
}

void UItemizationCoreStatics::GetIconComponentData(
	const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_Icon& OutData)
{
	if (ItemDefinition == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	const FItemComponentData_Icon* IconComp = ItemDefinition->GetDisplayComponent<FItemComponentData_Icon>();
	if (IconComp == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	OutData = *IconComp;
	bOutSuccess = true;
}

void UItemizationCoreStatics::GetInventoryItemLimitByTagComponentData(
	const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_InventoryItemLimitByTag& OutData)
{
	if (ItemDefinition == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	const FItemComponentData_InventoryItemLimitByTag* LimitComp = ItemDefinition->GetItemComponent<FItemComponentData_InventoryItemLimitByTag>();
	if (LimitComp == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	OutData = *LimitComp;
	bOutSuccess = true;
}

void UItemizationCoreStatics::GetMaxStackSizeComponentData(
	const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_MaxStackSize& OutData)
{
	if (ItemDefinition == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	const FItemComponentData_MaxStackSize* StackSizeComp = ItemDefinition->GetItemComponent<FItemComponentData_MaxStackSize>();
	if (StackSizeComp == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	OutData = *StackSizeComp;
	bOutSuccess = true;
}

void UItemizationCoreStatics::GetOwnedGameplayTagsComponentData(
	const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_OwnedGameplayTags& OutData)
{
	if (ItemDefinition == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	const FItemComponentData_OwnedGameplayTags* TagsComp = ItemDefinition->GetItemComponent<FItemComponentData_OwnedGameplayTags>();
	if (TagsComp == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	OutData = *TagsComp;
	bOutSuccess = true;
}

void UItemizationCoreStatics::GetSlotSizeComponentData(
	const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_SlotSize& OutData)
{
	if (ItemDefinition == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	const FItemComponentData_SlotSize* SlotSizeComp = ItemDefinition->GetItemComponent<FItemComponentData_SlotSize>();
	if (SlotSizeComp == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	OutData = *SlotSizeComp;
	bOutSuccess = true;
}

void UItemizationCoreStatics::GetTraitsComponentData(
	const UItemDefinition* ItemDefinition, bool& bOutSuccess, FItemComponentData_Traits& OutData)
{
	if (ItemDefinition == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	const FItemComponentData_Traits* TraitsComp = ItemDefinition->GetItemComponent<FItemComponentData_Traits>();
	if (TraitsComp == nullptr)
	{
		bOutSuccess = false;
		return;
	}

	OutData = *TraitsComp;
	bOutSuccess = true;
}
