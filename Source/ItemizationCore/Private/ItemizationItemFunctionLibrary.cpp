// Author: Tom Werner (MajorT), 2025


#include "ItemizationItemFunctionLibrary.h"

#include "ItemizationCoreLogChannels.h"
#include "Items/ItemDefinitionBase.h"
#include "Items/Data/ItemComponentData_Icon.h"
#include "Items/Data/ItemComponentData_MaxStackSize.h"
#include "Items/Data/ItemComponentData_Traits.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemizationItemFunctionLibrary)

#define ITEM_DEFINITION_VALID_OR_RETURN(ReturnType) \
	if (!IsValid(ItemDefinition)) return ReturnType;

bool UItemizationItemFunctionLibrary::ItemHasTrait(const UItemDefinitionBase* ItemDefinition, FGameplayTag TraitToCheck)
{
	ITEM_DEFINITION_VALID_OR_RETURN(false)
	
	return FItemComponentData_Traits::HasTrait(ItemDefinition, TraitToCheck);
}

bool UItemizationItemFunctionLibrary::IsItemStackable(const UItemDefinitionBase* ItemDefinition)
{
	ITEM_DEFINITION_VALID_OR_RETURN(false)
	
	const FItemComponentData_MaxStackSize* MaxStackSizeData = ItemDefinition->GetItemData<FItemComponentData_MaxStackSize>();
	if (MaxStackSizeData == nullptr)
	{
		return false;
	}

	return MaxStackSizeData->GetMaxStackSize() > 1;
}

int32 UItemizationItemFunctionLibrary::GetItemMaxStackSize(const UItemDefinitionBase* ItemDefinition)
{
	ITEM_DEFINITION_VALID_OR_RETURN(1)

	const FItemComponentData_MaxStackSize* MaxStackSizeData = ItemDefinition->GetItemData<FItemComponentData_MaxStackSize>();
	if (MaxStackSizeData == nullptr)
	{
		ITEMIZATION_WARN("No MaxStackSizeData found for item definition '%s'. Returning default value of 1.", *ItemDefinition->GetName());
		return 1;
	}

	return MaxStackSizeData->GetMaxStackSize();
}

TSoftObjectPtr<UTexture2D> UItemizationItemFunctionLibrary::GetItemIcon(const UItemDefinitionBase* ItemDefinition)
{
	ITEM_DEFINITION_VALID_OR_RETURN(nullptr);

	const FItemComponentData_Icon* IconData = ItemDefinition->GetItemData<FItemComponentData_Icon>();
	if (IconData == nullptr)
	{
		ITEMIZATION_WARN("No IconData found for item definition '%s'. Returning null.", *ItemDefinition->GetName());
		return nullptr;
	}

	return IconData->Icon;
}

FText UItemizationItemFunctionLibrary::GetItemName(const UItemDefinitionBase* ItemDefinition)
{
	ITEM_DEFINITION_VALID_OR_RETURN(FText::GetEmpty())
	return ItemDefinition->GetItemName();
}

FText UItemizationItemFunctionLibrary::GetItemDescription(const UItemDefinitionBase* ItemDefinition, bool bFallbackToShort)
{
	ITEM_DEFINITION_VALID_OR_RETURN(FText::GetEmpty())
	return ItemDefinition->GetItemDescription(bFallbackToShort);
}

FText UItemizationItemFunctionLibrary::GetItemRichDescription(const UItemDefinitionBase* ItemDefinition, bool bFallbackToShort)
{
	ITEM_DEFINITION_VALID_OR_RETURN(FText::GetEmpty())
	return ItemDefinition->GetItemRichDescription();
}

FText UItemizationItemFunctionLibrary::GetItemTypeName(const UItemDefinitionBase* ItemDefinition)
{
	ITEM_DEFINITION_VALID_OR_RETURN(FText::GetEmpty())
	return FText::FromString(ItemDefinition->GetPrimaryAssetId().PrimaryAssetType.ToString());
}
