// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemizationUtilities.h"

#include "ItemDefinition.h"
#include "Components/ItemComponentData_Icon.h"

FSoftObjectPath UE::ItemizationCore::Display::FindItemIcon(const UItemDefinition* InItem)
{
	if (InItem == nullptr)
	{
		return FSoftObjectPath();
	}

	if (const FItemComponentData_Icon* IconComponent = InItem->GetDisplayComponent<FItemComponentData_Icon>())
	{
		return IconComponent->Icon.ToSoftObjectPath();
	}

	return FSoftObjectPath();
}

FText UE::ItemizationCore::Display::GatherItemDesc_Any(const UItemDefinition* InItem)
{
	if (InItem == nullptr)
	{
		return FText::GetEmpty();
	}

	if (!InItem->ItemDescription.IsEmpty())
	{
		return InItem->ItemDescription;
	}

	if (!InItem->ItemShortDescription.IsEmpty())
	{
		return InItem->ItemShortDescription;
	}

	return FText::GetEmpty();
}

FText UE::ItemizationCore::Display::GatherItemDisplayName(const UItemDefinition* InItem)
{
	if (InItem == nullptr)
	{
		return FText::GetEmpty();
	}

	if (!InItem->ItemName.IsEmpty())
	{
		return InItem->ItemName;
	}

	return FText::FromString(GetNameSafe(InItem));
}