// Copyright Epic Games, Inc. All Rights Reserved.


#include "Components/ItemComponentData_InventoryItemLimitByTag.h"

#include "ItemDefinition.h"
#include "ActorComponents/InventoryManager.h"
#include "Misc/DataValidation.h"


FItemComponentData_InventoryItemLimitByTag::FItemComponentData_InventoryItemLimitByTag()
{
	LimitAmount.Value = 0.f;
}

void FItemComponentData_InventoryItemLimitByTag::EvaluateContext(const FInventoryItemEntry& ItemEntry, FItemActionContextData& Context) const
{
	if (LimitTag.IsValid() && Context.ContextTags)
	{
		Context.ContextTags->AddTag(LimitTag);
	}
}

bool FItemComponentData_InventoryItemLimitByTag::CanCreateNewStack(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& Context) const
{
	if (!LimitTag.IsValid())
	{
		return true;
	}

	if (LimitAmount.Value <= 0.f)
	{
		return true;
	}
	
	check(Context.InventoryManager.IsValid());
	UInventoryManager* InventoryManager = Context.InventoryManager.Get();

	int32 MyTagCount = 0;
	for (const FInventoryItemEntry& Item : InventoryManager->GetInventoryList())
	{
		if (const FItemComponentData_InventoryItemLimitByTag* Data_Limit = Item.Definition->GetItemComponent<FItemComponentData_InventoryItemLimitByTag>())
		{
			if (Data_Limit->LimitTag == LimitTag)
			{
				MyTagCount++;
			}
		}
	}

	if (LimitAmount.AsInteger() > MyTagCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanCreateNewStack: LimitAmount: %d, MyTagCount: %d"), LimitAmount.AsInteger(), MyTagCount);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CanCreateNewStack: LimitAmount: %d, MyTagCount: %d"), LimitAmount.AsInteger(), MyTagCount);
	}

	return LimitAmount.AsInteger() > MyTagCount;
}

#if WITH_EDITOR
EDataValidationResult FItemComponentData_InventoryItemLimitByTag::IsDataValid(FDataValidationContext& Context) const
{
	if (!LimitTag.IsValid())
	{
		Context.AddError( NSLOCTEXT("ItemizationCore", "InventoryItemLimitByTag_InvalidTag", "Inventory Item Limit By Tag: Has an invalid tag."));
		return EDataValidationResult::Invalid;
	}

	return Super::IsDataValid(Context);
}
#endif