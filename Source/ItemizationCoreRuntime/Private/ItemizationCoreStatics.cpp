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
