// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationCoreCheatManagerExtension.h"

#include "ItemDefinition.h"
#include "ActorComponents/InventoryManager.h"
#include "Engine/AssetManager.h"
#include "Engine/Console.h"
#include "Engine/AssetUserData.h"
#include "AssetRegistry/AssetData.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"

UItemizationCoreCheatManagerExtension::UItemizationCoreCheatManagerExtension()
{
#if WITH_SERVER_CODE && UE_WITH_CHEAT_MANAGER
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(
			[](UCheatManager* CheatManager)
		{
			CheatManager->AddCheatManagerExtension(NewObject<ThisClass>(CheatManager));
		}));
	}
#endif
#if ALLOW_CONSOLE
	UConsole::RegisterConsoleAutoCompleteEntries.AddStatic(&UItemizationCoreCheatManagerExtension::PopulateAutoCompleteEntries);
#endif
}

void UItemizationCoreCheatManagerExtension::PopulateAutoCompleteEntries(TArray<FAutoCompleteCommand>& AutoCompleteCommands)
{
#if UE_WITH_CHEAT_MANAGER
	/*static bool bInitialized = false;
	if (bInitialized)
	{
		return;
	}
	bInitialized = true;*/
	
	TArray<FAssetData> Assets;

	TArray<FName> AssetTypes;
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(UItemDefinition::StaticClass(), DerivedClasses, true);
	DerivedClasses.Add(UItemDefinition::StaticClass());
	
	for (const UClass* Class : DerivedClasses)
	{
		if (const UItemDefinition* CDO = Class->GetDefaultObject<UItemDefinition>())
		{
			AssetTypes.AddUnique( CDO->GetPrimaryAssetId().PrimaryAssetType);
		}
	}

	// Get all item definitions that were found
	for (const auto AssetType : AssetTypes)
	{
		UAssetManager::Get().GetPrimaryAssetDataList(AssetType, Assets);
	}
	
	const UConsoleSettings* ConsoleSettings = GetDefault<UConsoleSettings>();
	
	for (const FAssetData& Asset : Assets)
	{
		if (!Asset.GetAsset()->IsA<UItemDefinition>())
		{
			continue;
		}

		FString PathName = Asset.PackageName.ToString();
		PathName = PathName.RightChop(PathName.Find(TEXT("/"), ESearchCase::CaseSensitive, ESearchDir::FromEnd) + 1);

		FAutoCompleteCommand Cmd
		(
			*FString::Printf(TEXT("GiveItem %s"), *Asset.PackageName.ToString()),
			*FString::Printf(TEXT("Gives an item of type %s."), *PathName),
			ConsoleSettings->AutoCompleteCommandColor
		);

		auto ContainsPred = [Cmd](const FAutoCompleteCommand& Existing) -> bool
		{
			if (Existing.Command == Cmd.Command)
			{
				return true;
			}
			
			return false;
		};
		
		if (AutoCompleteCommands.ContainsByPredicate(ContainsPred))
		{
			continue;
		}
		
		AutoCompleteCommands.Add(MoveTemp(Cmd));
	}
#endif
}

void UItemizationCoreCheatManagerExtension::GiveItem(FString ItemPath, int32 StackCount)
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetPlayerController();
	UInventoryManager* InventoryManager = UInventoryManager::FindInventoryManager(PC);
	if (!InventoryManager)
	{
		UE_LOG(LogConsoleResponse, Log, TEXT("%s does not have an InventoryManager component."), *GetNameSafe(PC));
		return;
	}

	if (ItemPath.IsEmpty())
	{
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Attempting to give item %s (count: %d) to %s."), *ItemPath, StackCount, *GetNameSafe(PC));

	UItemDefinition* ItemDefinition = Cast<UItemDefinition>(StaticLoadObject(UItemDefinition::StaticClass(), nullptr, *ItemPath));
	if (!ItemDefinition)
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to load item definition %s."), *ItemPath);
		return;
	}

	FInventoryItemEntry NewEntry(ItemDefinition, StackCount, PC);

	int32 Excess = 0;
	InventoryManager->GiveItem(NewEntry, Excess);

	if (Excess > 0)
	{
		UE_LOG(LogConsoleResponse, Log, TEXT("Excess items: %d"), Excess);
	}
#endif
}
