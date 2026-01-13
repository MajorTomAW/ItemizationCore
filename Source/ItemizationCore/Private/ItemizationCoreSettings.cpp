// Author: Tom Werner (MajorT), 2025


#include "ItemizationCoreSettings.h"

#include "ItemizationCoreLogChannels.h"
#include "ItemizationCoreTags.h"

UItemizationCoreSettings::UItemizationCoreSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AllowEmptyStackTag = Itemization::Tags::TAG_ItemTrait_AllowEmptyStack;
	HideToastTag = Itemization::Tags::TAG_ItemTrait_HideItemToast;
	IgnoreRemoveAllTag = Itemization::Tags::TAG_ItemTrait_IgnoreRemoveAllInventoryItems;
	CountTowardsLimitTag = Itemization::Tags::TAG_ItemTrait_InventorySizeLimited;
	SingleStackTag = Itemization::Tags::TAG_ItemTrait_SingleStack;
	TransientTag = Itemization::Tags::TAG_ItemTrait_Transient;
	AutoCombineStacks = Itemization::Tags::TAG_ItemTrait_AutoCombineStacks;

	ItemTypes.Append({NAME_None, "ItemDefinition"});
}

const UItemizationCoreSettings* UItemizationCoreSettings::Get()
{
	return GetDefault<UItemizationCoreSettings>();
}

UItemizationCoreSettings* UItemizationCoreSettings::GetMutable()
{
	return GetMutableDefault<UItemizationCoreSettings>();
}

TArray<FName> UItemizationCoreSettings::GetItemTypes()
{
	return Get()->ItemTypes;
}

#if WITH_EDITOR
void UItemizationCoreSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, ItemTypes))
	{
		// We don't like to have dupes of item types, so log a warning if we find any.
		TSet<FName> UniqueItemTypes;
		for (const FName& ItemType : ItemTypes)
		{
			if (UniqueItemTypes.Contains(ItemType) && ItemType != NAME_None)
			{
				ITEMIZATION_ERROR("Duplicate item type found: %s", *ItemType.ToString());
				continue;
			}

			UniqueItemTypes.Add(ItemType);
		}
	}
}
#endif
