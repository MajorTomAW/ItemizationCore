// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "ConsoleSettings.h"
#include "GameFramework/CheatManager.h"
#include "ItemizationCheatManagerExtension.generated.h"

/** Cheats for the itemization core */
UCLASS(NotBlueprintable)
class UItemizationCheatManagerExtension final : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	UItemizationCheatManagerExtension();

	//~ Begin UCheatManagerExtension Interface
	virtual void AddedToCheatManager_Implementation() override;
	//~ End UCheatManagerExtension Interface

protected:
	static void PopulateAutoCompleteEntries(TArray<FAutoCompleteCommand>& AutoCompleteCommands);

	/**
	 * Gives a specified item and count to the owning player.
	 * @param ItemAssetId PrimaryAssetId and Name of the item.
	 * @param Count The number of items to give. Default is 1.
	 */
	UFUNCTION(Exec)
	void GiveItem(const FString& ItemAssetId, int32 Count = 1) const;
};
