// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemAssetFactory.h"

#include "ItemDefinition.h"

#define LOCTEXT_NAMESPACE "ItemAssetFactory"

UItemAssetFactory::UItemAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UItemDefinition::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

bool UItemAssetFactory::ConfigureProperties()
{
	return Super::ConfigureProperties();
}

FText UItemAssetFactory::GetDisplayName() const
{
	return LOCTEXT("ItemAssetFactoryDisplayName", "Item Definition");
}

FString UItemAssetFactory::GetDefaultNewAssetName() const
{
	return FString(TEXT("New")) + UItemDefinition::StaticClass()->GetName();
}

UObject* UItemAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(InClass->IsChildOf(UItemDefinition::StaticClass()));
	return NewObject<UItemDefinition>(InParent, InClass, InName, Flags);
}

#undef LOCTEXT_NAMESPACE