// Author: Tom Werner (dc: majort), 2026 January


#include "Cheats/ItemizationCheatManagerExtension.h"

#include "ItemizationCoreSettings.h"
#include "Components/InventoryComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/Console.h"
#include "Inventory/InventoryBase.h"
#include "Items/ItemDefinitionBase.h"
#include "Items/IInventoryItemInstanceInterface.h"


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
			AutoCompleteCmd.Desc = FString::Printf(TEXT("Gives %s to the owning player."), *AssetData.GetPrimaryAssetId().PrimaryAssetName.ToString());
			AutoCompleteCmd.Color = ConsoleSettings->InputColor;
			AutoCompleteCommands.Add(AutoCompleteCmd);
		}

		// Remove Command
		{
			FAutoCompleteCommand AutoCompleteCmd;
			AutoCompleteCmd.Command = FString::Printf(TEXT("RemoveItem %s"), *AssetData.GetPrimaryAssetId().ToString());
			AutoCompleteCmd.Desc = FString::Printf(TEXT("Removes %s from the owning player."), *AssetData.GetPrimaryAssetId().PrimaryAssetName.ToString());
			AutoCompleteCmd.Color = ConsoleSettings->InputColor;
			AutoCompleteCommands.Add(AutoCompleteCmd);
		}
	}
#endif
}

void UItemizationCheatManagerExtension::GiveItem(const FString& ItemAssetId, int32 Count) const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	UInventoryComponent* InventoryComp = PC ? PC->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!IsValid(InventoryComp))
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("%s does not have an Inventory Component."), *GetNameSafe(PC))
		return;
	}

	UItemDefinitionBase* ItemDef = FindItemDefinition(ItemAssetId);
	if (!IsValid(ItemDef))
	{
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Giving Item %s (count: %d) to %s"), *ItemAssetId, Count, *PC->GetName())

	int32 Excess;
	InventoryComp->TryGiveItem(ItemDef, Count, PC, Itemization::Tags::TAG_InventoryGroup_Inventory, Excess);
#endif
}

void UItemizationCheatManagerExtension::RemoveItem(const FString& ItemAssetId, int32 Count) const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	UInventoryComponent* InventoryComp = PC ? PC->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!IsValid(InventoryComp))
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("%s does not have an Inventory Component."), *GetNameSafe(PC))
		return;
	}

	UItemDefinitionBase* ItemDef = FindItemDefinition(ItemAssetId);
	if (!IsValid(ItemDef))
	{
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Removing Item %s (count: %d) from %s"), *ItemAssetId, Count, *PC->GetName())

	InventoryComp->TryRemoveItemByDefinition(ItemDef, Count, FGameplayTag());
#endif
}

void UItemizationCheatManagerExtension::RemoveItemById(uint32 ItemId, int32 Count) const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	UInventoryComponent* InventoryComp = PC ? PC->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!IsValid(InventoryComp))
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("%s does not have an Inventory Component."), *GetNameSafe(PC))
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Removing Item %u (count: %d) from %s"), ItemId, Count, *PC->GetName())

	InventoryComp->TryRemoveItemById(FInventoryItemId(ItemId), Count, FGameplayTag());
#endif
}

void UItemizationCheatManagerExtension::DropItem(const FString& ItemAssetId, int32 Count) const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	UInventoryComponent* InventoryComp = PC ? PC->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!IsValid(InventoryComp))
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("%s does not have an Inventory Component."), *GetNameSafe(PC))
		return;
	}

	UItemDefinitionBase* ItemDef = FindItemDefinition(ItemAssetId);
	if (!IsValid(ItemDef))
	{
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Dropping item %s (count: %d) from %s"), *ItemAssetId, Count, *PC->GetName())

	//@TODO: Use inventory comp wrapper for this
	InventoryComp->GetInventory()->DropItem(ItemDef, Count);
#endif
}

void UItemizationCheatManagerExtension::DropItemById(uint32 ItemId, int32 Count) const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	UInventoryComponent* InventoryComp = PC ? PC->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!IsValid(InventoryComp))
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("%s does not have an Inventory Component."), *GetNameSafe(PC))
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Dropping item %u (count: %d) from %s"), ItemId, Count, *PC->GetName())

	//@TODO: Use inventory comp wrapper for this
	InventoryComp->GetInventory()->DropItem(FInventoryItemId(ItemId), Count);
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
