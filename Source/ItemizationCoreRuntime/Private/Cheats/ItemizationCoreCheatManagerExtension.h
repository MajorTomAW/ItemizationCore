// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ConsoleSettings.h"
#include "GameFramework/CheatManager.h"
#include "ItemizationCoreCheatManagerExtension.generated.h"

/**
 * Cheats related to Itemization Core / Inventory System.
 */
UCLASS(NotBlueprintable)
class UItemizationCoreCheatManagerExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	UItemizationCoreCheatManagerExtension();

protected:
	static void PopulateAutoCompleteEntries(TArray<FAutoCompleteCommand>& AutoCompleteCommands);

	/**
	 * @param ItemPath The path to the item to give. 
	 * @param StackCount  The number of items to give. Default is 1.
	 */
	UFUNCTION(Exec)
	void GiveItem(FString ItemPath, int32 StackCount = 1);
};
