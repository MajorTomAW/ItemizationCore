// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "ItemDefinition.h"

class FItemizationEditorApplication;

struct FItemizationDetailsViewArgs
{
public:
	friend class FItemizationDetailsCustomization;

	static FItemizationDetailsViewArgs DetailsView()
	{
		FItemizationDetailsViewArgs Args;
		Args.PropertyBlackList.Add(GET_MEMBER_NAME_CHECKED(UItemDefinition, ItemComponents));
		Args.PropertyBlackList.Add(GET_MEMBER_NAME_CHECKED(UItemDefinition, EquipmentData));

		Args.CategoryBlackList.Append(DisplayInfo().CategoryWhiteList);
		Args.CategoryBlackList.Append(Placement().CategoryWhiteList);
		Args.CategoryBlackList.Append(Development().CategoryWhiteList);
		
		return Args;
	}

	static FItemizationDetailsViewArgs DisplayInfo()
	{
		FItemizationDetailsViewArgs Args;
		Args.CategoryWhiteList.Add("Display");
		Args.PropertyBlackList.Add(GET_MEMBER_NAME_CHECKED(UItemDefinition, ItemComponents));
		Args.PropertyBlackList.Add(GET_MEMBER_NAME_CHECKED(UItemDefinition, EquipmentData));
		return Args;
	}

	static FItemizationDetailsViewArgs Placement()
	{
		FItemizationDetailsViewArgs Args;
		Args.CategoryWhiteList.Add("Placement");
		Args.PropertyBlackList.Add(GET_MEMBER_NAME_CHECKED(UItemDefinition, ItemComponents));
		Args.PropertyBlackList.Add(GET_MEMBER_NAME_CHECKED(UItemDefinition, EquipmentData));
		return Args;
	}

	static FItemizationDetailsViewArgs Development()
	{
		FItemizationDetailsViewArgs Args;
		Args.CategoryWhiteList.Add("Development");
		Args.PropertyBlackList.Add(GET_MEMBER_NAME_CHECKED(UItemDefinition, ItemComponents));
		Args.PropertyBlackList.Add(GET_MEMBER_NAME_CHECKED(UItemDefinition, EquipmentData));
		return Args;
	}

	static FItemizationDetailsViewArgs Components()
	{
		FItemizationDetailsViewArgs Args;
		Args.CategoryWhiteList.Add("Components");
		Args.PropertyBlackList.Add(GET_MEMBER_NAME_CHECKED(UItemDefinition, EquipmentData));
		return Args;
	}

	static FItemizationDetailsViewArgs Equipment()
	{
		FItemizationDetailsViewArgs Args;
		Args.CategoryWhiteList.Add("Equipment");
		return Args;
	}

private:
	TArray<FName> CategoryBlackList;
	TArray<FName> PropertyBlackList;
	TArray<FName> CategoryWhiteList;
};

class FItemizationDetailsCustomization : public IDetailCustomization
{
public:
	FItemizationDetailsCustomization();
	static TSharedRef<IDetailCustomization> MakeInstance(const FItemizationDetailsViewArgs& InArgs, const TSharedPtr<FItemizationEditorApplication>& InApp);

protected:
	//~ Begin IDetailCustomization Interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//~ End IDetailCustomization Interface

protected:
	FItemizationDetailsViewArgs Args;
	TWeakPtr<FItemizationEditorApplication> WeakApp;
};
