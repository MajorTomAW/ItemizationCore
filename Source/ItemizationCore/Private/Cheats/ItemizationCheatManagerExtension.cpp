// Author: Tom Werner (dc: majort), 2026 January


#include "Cheats/ItemizationCheatManagerExtension.h"

#include "InventoryLibrary.h"
#include "ItemizationCoreSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/Console.h"
#include "InventoryBase.h"
#include "Items/ItemDefinitionBase.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemizationCheatManagerExtension)

UItemizationCheatManagerExtension::UItemizationCheatManagerExtension()
{
#if WITH_SERVER_CODE && UE_WITH_CHEAT_MANAGER
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(
			[](UCheatManager* CheatManager)
			{
				CheatManager->AddCheatManagerExtension(NewObject<ThisClass>(CheatManager));
			}));

#if ALLOW_CONSOLE
		UConsole::RegisterConsoleAutoCompleteEntries.AddStatic(&ThisClass::PopulateAutoCompleteEntries);
#endif
	}
#endif
}

void UItemizationCheatManagerExtension::AddedToCheatManager_Implementation()
{
	Super::AddedToCheatManager_Implementation();
}

void UItemizationCheatManagerExtension::PopulateAutoCompleteEntries(TArray<FAutoCompleteCommand>& AutoCompleteCommands)
{
#if UE_WITH_CHEAT_MANAGER
	// Get asset types from settings
	TArray<FName> AssetTypes = UItemizationCoreSettings::Get()->ItemTypes;

	// Find all primary data assets
	TArray<FAssetData> Assets;
	for (const FName& AssetType : AssetTypes)
	{
		UAssetManager::Get().GetPrimaryAssetDataList(AssetType, Assets);
	}

	const UConsoleSettings* ConsoleSettings = GetDefault<UConsoleSettings>();
	for (const FAssetData& AssetData : Assets)
	{
		if (!AssetData.GetAsset()->IsA<UItemDefinitionBase>())
		{
			continue;
		}

		// Give Command
		{
			FAutoCompleteCommand AutoCompleteCmd;
			AutoCompleteCmd.Command = FString::Printf(TEXT("GiveItem %s"), *AssetData.GetPrimaryAssetId().ToString());
			AutoCompleteCmd.Desc = FString::Printf(TEXT("ItemAssetId[FString] Count[int32] GroupName[FString]"));
			AutoCompleteCmd.Color = ConsoleSettings->InputColor;
			AutoCompleteCommands.Add(AutoCompleteCmd);
		}

		// Remove Command
		{
			FAutoCompleteCommand AutoCompleteCmd;
			AutoCompleteCmd.Command = FString::Printf(TEXT("RemoveItem %s"), *AssetData.GetPrimaryAssetId().ToString());
			AutoCompleteCmd.Desc = FString::Printf(TEXT("ItemAssetId[FString] Count[int32]"));
			AutoCompleteCmd.Color = ConsoleSettings->InputColor;
			AutoCompleteCommands.Add(AutoCompleteCmd);
		}
	}
#endif
}

void UItemizationCheatManagerExtension::GiveItem(
	const FString& ItemAssetId,
	int32 Count,
	FString GroupName) const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	AInventoryBase* Inventory = FindInventory(PC);
	if (!IsValid(Inventory))
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("%s does not have an Inventory."), *GetNameSafe(PC))
		return;
	}

	UItemDefinitionBase* ItemDef = FindItemDefinition(ItemAssetId);
	if (!IsValid(ItemDef))
	{
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Giving Item %s (count: %d) to %s"), *ItemAssetId, Count, *PC->GetName())

	FGameplayTag GroupTag;
	if (!GroupName.IsEmpty())
	{
		GroupTag = FGameplayTag::RequestGameplayTag(*GroupName);
	}

	int32 Excess;
	Inventory->GiveItem(ItemDef, Count, PC, GroupTag, Excess);
#endif
}

void UItemizationCheatManagerExtension::RemoveItem(
	const FString& ItemAssetId,
	int32 Count) const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	AInventoryBase* Inventory = FindInventory(PC);
	if (!IsValid(Inventory))
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("%s does not have an Inventory."), *GetNameSafe(PC))
		return;
	}

	UItemDefinitionBase* ItemDef = FindItemDefinition(ItemAssetId);
	if (!IsValid(ItemDef))
	{
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Removing Item %s (count: %d) from %s"), *ItemAssetId, Count, *PC->GetName())

	//Inventory->RemoveItemByDefinition(ItemDef, Count, GroupTag);
#endif
}

void UItemizationCheatManagerExtension::RemoveItemById(uint32 ItemId, int32 Count) const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	AInventoryBase* Inventory = FindInventory(PC);
	if (!IsValid(Inventory))
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("%s does not have an Inventory."), *GetNameSafe(PC))
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Removing Item %u (count: %d) from %s"), ItemId, Count, *PC->GetName())

	Inventory->RemoveItemById(FInventoryItemId(ItemId), Count);
#endif
}

void UItemizationCheatManagerExtension::SwapItemSlots(
	uint32 SlotRowA, uint32 SlotColA, const FName& GroupNameA,
	uint32 SlotRowB, uint32 SlotColB, const FName& GroupNameB) const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	AInventoryBase* Inventory = FindInventory(PC);
	if (!IsValid(Inventory))
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("%s does not have an Inventory."), *GetNameSafe(PC))
		return;
	}

	FInventorySlotId SlotA(SlotRowA, SlotColA);
	FInventorySlotId SlotB(SlotRowB, SlotColB);

	FGameplayTag GroupTagA;
	if (!GroupNameA.IsNone())
	{
		GroupTagA = FGameplayTag::RequestGameplayTag(GroupNameA);
	}
	FGameplayTag GroupTagB;
	if (!GroupNameB.IsNone())
	{
		GroupTagB = FGameplayTag::RequestGameplayTag(GroupNameB);
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Swapping slot '%s' [%s] with '%s' [%s]"),
		*SlotA.ToString(), *GroupTagA.ToString(), *SlotB.ToString(), *GroupTagB.ToString())

	Inventory->SwapItemSlots(SlotA, GroupTagA, SlotB, GroupTagB);
#endif
}

void UItemizationCheatManagerExtension::DropItem(const FString& ItemAssetId, int32 Count) const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	AInventoryBase* Inventory = FindInventory(PC);
	if (!IsValid(Inventory))
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("%s does not have an Inventory."), *GetNameSafe(PC))
		return;
	}

	UItemDefinitionBase* ItemDef = FindItemDefinition(ItemAssetId);
	if (!IsValid(ItemDef))
	{
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Dropping item %s (count: %d) from %s"), *ItemAssetId, Count, *PC->GetName())


	//Inventory->DropItem(ItemDef, Count);
#endif
}

void UItemizationCheatManagerExtension::DropItemById(uint32 ItemId, int32 Count) const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	AInventoryBase* Inventory = FindInventory(PC);
	if (!IsValid(Inventory))
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("%s does not have an Inventory."), *GetNameSafe(PC))
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Dropping item %u (count: %d) from %s"), ItemId, Count, *PC->GetName())

	Inventory->DropItem(FInventoryItemId(ItemId), Count);
#endif
}

UItemDefinitionBase* UItemizationCheatManagerExtension::FindItemDefinition(const FString& ItemAssetId) const
{
	if (ItemAssetId.IsEmpty())
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Passed in an empty Item Asset Id"))
		return nullptr;
	}

	FPrimaryAssetId PrimaryAssetId(ItemAssetId);
	if (!PrimaryAssetId.IsValid())
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Item Asset Id (%s) is valid but couldn't be resolved into a PrimaryAssetId"), *ItemAssetId)
		return nullptr;
	}

	const FSoftObjectPath ItemPath = UAssetManager::Get().GetPrimaryAssetPath(PrimaryAssetId);
	const FString ItemPathStr = ItemPath.ToString();

	if (!ItemPath.IsValid())
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Unable to find item path for item %s"), *PrimaryAssetId.ToString())
		return nullptr;
	}

	UItemDefinitionBase* ItemDef = Cast<UItemDefinitionBase>(StaticLoadObject(
		UItemDefinitionBase::StaticClass(),
		nullptr,
		*ItemPathStr));

	if (!IsValid(ItemDef))
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to load item %s."), *ItemPathStr)
	}

	return ItemDef;
}

AInventoryBase* UItemizationCheatManagerExtension::FindInventory(AActor* Actor) const
{
	return UInventoryLibrary::FindInventory(Actor);
}
