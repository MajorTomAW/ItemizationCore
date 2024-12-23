// Copyright Epic Games, Inc. All Rights Reserved.


#include "Components/ItemComponentData_MaxStackSize.h"

#include "ItemDefinition.h"
#include "ItemizationCoreLog.h"

FItemComponentData_MaxStackSize::FItemComponentData_MaxStackSize()
{
	MaxStackSize.Value = 1.f;
}

void FItemComponentData_MaxStackSize::EvaluateContext(
	const FInventoryItemEntry& ItemEntry, FItemActionContextData& Context) const
{
	Context.MaxStackSize = MaxStackSize.AsInteger();
}

bool FItemComponentData_MaxStackSize::CanCombineItemStacks(
	const FInventoryItemEntry& ThisEntry, const FInventoryItemEntry& OtherEntry, FItemActionContextData& Context) const
{
	ensureMsgf(MaxStackSize.AsInteger() == Context.MaxStackSize, TEXT("MaxStackSize value mismatch between item definition and context data."));

	if (!ensure(OtherEntry.Instance))
	{
		ITEMIZATION_LOG(Error, TEXT("CanCombineItemStacks: OtherEntry (%s) has no instance."), *GetNameSafe(OtherEntry.Definition));
		return false;
	}

	// Less or equal to 0 means infinite stack size.
	if (Context.MaxStackSize <= 0)
	{
		return true;
	}
	
	return OtherEntry.StackCount < Context.MaxStackSize;
}
