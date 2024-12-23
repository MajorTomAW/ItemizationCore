// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationCoreTypes.h"

#include "InventoryItemInstance.h"
#include "ActorComponents/InventoryManager.h"
#include "GameFramework/PlayerState.h"

FString FItemActionContextData::ToString() const
{
	return FString::Printf(TEXT("Context Data:\n    Instigator: %s\n    InventoryManager: %s\n    ContextTags: %s\n    Delta: %d\n    SlotSize: %d\n    MaxStackSize: %d"),
			*GetNameSafe(Instigator.Get()), *GetNameSafe(InventoryManager.Get()), ContextTags ? *ContextTags->ToString() : TEXT("Null"), Delta, SlotSize, MaxStackSize);
}

FInventoryChangeMessage::FInventoryChangeMessage(UInventoryItemInstance* ItemThatChanged, const int32 OldCount, const int32 NewCount)
{
	Item = ItemThatChanged;
	NewStackCount = NewCount;
	OldStackCount = OldCount;
	Delta = NewCount - OldCount;
	InventoryManager = ItemThatChanged ? ItemThatChanged->GetCurrentInventoryManager() : nullptr;
	Owner = ItemThatChanged ? ItemThatChanged->GetTypedOuter<AActor>() : nullptr;

	if (AController* C = Cast<AController>(Owner))
	{
		Controller = C;
	}
	else if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		Controller = Pawn->GetController();
	}
	else if (const APlayerState* PS = Cast<APlayerState>(Owner))
	{
		Controller = PS->GetOwningController();
	}
}
