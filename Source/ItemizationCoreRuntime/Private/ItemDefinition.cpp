// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "InventoryItemInstance.h"
#include "Components/EquipmentComponentData_Default.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemDefinition)

#define LOCTEXT_NAMESPACE "ItemizationCore"

UItemDefinition::UItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PrimaryAssetType(TEXT("ItemDefinition"))
	, PrimaryAssetName(GetFName())
	, bAdjustForWorldCollision(false)
	, DefaultInstanceClass(UInventoryItemInstance::StaticClass())
{
	EquipmentData.InitializeAs<FEquipmentComponentData_Default>();
}

FPrimaryAssetId UItemDefinition::GetPrimaryAssetId() const
{
	if (PrimaryAssetType.IsNone() || PrimaryAssetName.IsNone())
	{
		return FPrimaryAssetId(TEXT("ItemDefinition"), GetFName());
	}
	
	return FPrimaryAssetId(PrimaryAssetType, PrimaryAssetName);
}

TSubclassOf<UInventoryItemInstance> UItemDefinition::GetDefaultInstanceClass() const
{
	if (DefaultInstanceClass.IsNull())
	{
		return UInventoryItemInstance::StaticClass();
	}

	return DefaultInstanceClass.LoadSynchronous();
}

TSubclassOf<UInventoryEquipmentInstance> UItemDefinition::GetDefaultEquipmentInstanceClass() const
{
	if (DefaultEquipmentInstanceClass.IsNull())
	{
		return UInventoryEquipmentInstance::StaticClass();
	}

	return DefaultEquipmentInstanceClass.LoadSynchronous();
}

TArray<const FItemComponentData*> UItemDefinition::GetDisplayComponents() const
{
	TArray<const FItemComponentData*> OutComponents;
	for (const auto& Proxy : DisplayComponents)
	{
		if (!Proxy.Component.IsValid())
		{
			continue;
		}
		
		if (const FItemComponentData* Component = &Proxy.Component.Get())
		{
			OutComponents.Add(Component);
		}
	}

	return OutComponents;
}

TArray<const FItemComponentData*> UItemDefinition::GetItemComponents() const
{
	TArray<const FItemComponentData*> OutComponents;
	for (const auto& Proxy : ItemComponents)
	{
		if (!Proxy.Component.IsValid())
		{
			continue;
		}
		
		if (const FItemComponentData* Component = &Proxy.Component.Get())
		{
			OutComponents.Add(Component);
		}
	}
	
	return OutComponents;
}

#if WITH_EDITOR

EDataValidationResult UItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (PrimaryAssetType.IsNone())
	{
		Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
		Context.AddError
		(
			FText::Format
			(
				LOCTEXT("ItemDefinition_InvalidPrimaryAssetType", "ItemDefinition '{0}' has an invalid PrimaryAssetType."),
				FText::FromName(GetFName())
			)
		);
	}

	if (PrimaryAssetName.IsNone())
	{
		Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
		Context.AddError
		(
			FText::Format
			(
				LOCTEXT("ItemDefinition_InvalidPrimaryAssetName", "ItemDefinition '{0}' has an invalid PrimaryAssetName."),
				FText::FromName(GetFName())
			)
		);
	}

	TArray<const FItemComponentData*> Components = GetItemComponents();
	Components.Append(GetDisplayComponents());
	
	for (const FItemComponentData* Component : Components)
	{
		if (Component != nullptr)
		{
			Result = CombineDataValidationResults(Result, Component->IsDataValid(Context));
		}
		else
		{
			Context.AddError
			(
				FText::Format
				(
					LOCTEXT("ItemDefinition_InvalidComponent", "ItemDefinition '{0}' has an invalid component."),
					FText::FromName(GetFName())
				)
			);
		}
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE