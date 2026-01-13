// Author: Tom Werner (dc: majort), 2026 January


#include "Cheats/ItemizationCheatManagerExtension.h"

#include "ItemizationCoreSettings.h"
#include "Components/InventoryComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/Console.h"
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

		FAutoCompleteCommand AutoCompleteCmd;
		AutoCompleteCmd.Command = FString::Printf(TEXT("GiveItem %s"), *AssetData.GetPrimaryAssetId().ToString());
		AutoCompleteCmd.Desc = FString::Printf(TEXT("Gives %s to the owning player."), *AssetData.GetPrimaryAssetId().PrimaryAssetName.ToString());
		AutoCompleteCmd.Color = ConsoleSettings->AutoCompleteCommandColor;

		AutoCompleteCommands.Add(AutoCompleteCmd);
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

	if (ItemAssetId.IsEmpty())
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Passed in an empty Item Asset Id"))
		return;
	}

	FPrimaryAssetId PrimaryAssetId(ItemAssetId);
	if (!PrimaryAssetId.IsValid())
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Item Asset Id (%s) is valid but couldn't be resolved into a PrimaryAssetId"), *ItemAssetId)
		return;
	}

	const FSoftObjectPath ItemPath = UAssetManager::Get().GetPrimaryAssetPath(PrimaryAssetId);
	const FString ItemPathStr = ItemPath.ToString();

	if (!ItemPath.IsValid())
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Unable to find item path for item %s"), *PrimaryAssetId.ToString())
		return;
	}

	UItemDefinitionBase* ItemDef = Cast<UItemDefinitionBase>(StaticLoadObject(
		UItemDefinitionBase::StaticClass(),
		nullptr,
		*ItemPathStr));

	if (!IsValid(ItemDef))
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to load item %s."), *ItemPathStr)
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Giving Item %s (count: %d) to %s"), *PrimaryAssetId.ToString(), Count, *PC->GetName())

	int32 Excess;
	InventoryComp->TryGiveItem(ItemDef, Count, PC, Itemization::Tags::TAG_InventoryGroup_Inventory, Excess);
#endif
}
