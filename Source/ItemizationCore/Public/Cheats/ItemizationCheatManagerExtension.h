// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "ConsoleSettings.h"
#include "GameFramework/CheatManager.h"

#include "ItemizationCheatManagerExtension.generated.h"

class UItemDefinitionBase;

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
	 * @param GroupName	String that will be resolved into a gameplay tag, representing the group to add the item to.
	 */
	UFUNCTION(Exec)
	void GiveItem(const FString& ItemAssetId, int32 Count = 1, const FName& GroupName = NAME_None) const;

	/**
	 * Removes the specified amount of the specified item from the owning player's inventory.
	 * @param ItemAssetId PrimaryAssetId and Name of the item,
	 * @param Count The number of items to remove. Default is 1.
	 * @param GroupName	String that will be resolved into a gameplay tag, representing the group to add the item to.
	 */
	UFUNCTION(Exec)
	void RemoveItem(const FString& ItemAssetId, int32 Count = 1, const FName& GroupName = NAME_None) const;

	/**
	 * Removes the specified amount of the specified item from the owning player's inventory.
	 * @param ItemId UID of the item to remove
	 * @param Count The number of items to remove. Default is 1.
	 */
	UFUNCTION(Exec)
	void RemoveItemById(uint32 ItemId, int32 Count = 1) const;

	/**
	 * Swaps the contents of the two specified slots
	 * @param SlotRowA The row of slot A
	 * @param SlotColA The column of slot A
	 * @param GroupNameA String that will be resoled into a gameplay tag, representing the group of slot A.
	 * @param SlotRowB The row of slot B
	 * @param SlotColB The column of slot B
	 * @param GroupNameB String that will be resoled into a gameplay tag, representing the group of slot B.
	 */
	UFUNCTION(Exec)
	void SwapItemSlots(uint32 SlotRowA, uint32 SlotColA, const FName& GroupNameA, uint32 SlotRowB, uint32 SlotColB, const FName& GroupNameB) const;

	/**
	 * Drops the specified amount of the specified item as a pickup into the world.
	 * @param ItemAssetId
	 * @param Count
	 */
	UFUNCTION(Exec)
	void DropItem(const FString& ItemAssetId, int32 Count = 1) const;

	/**
	 * Drops the specified amount of the specified item as a pickup into the world.
	 * @param ItemId UID of the item to drop
	 * @param Count
	 */
	UFUNCTION(Exec)
	void DropItemById(uint32 ItemId, int32 Count = 1) const;

private:
	UItemDefinitionBase* FindItemDefinition(const FString& ItemAssetId) const;
};
