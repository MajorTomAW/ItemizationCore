// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "ItemAssetFactory.generated.h"


class UItemDefinition;

UCLASS(HideCategories = Object)
class UItemAssetFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

protected:
	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual FText GetDisplayName() const override;
	virtual FString GetDefaultNewAssetName() const override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	//~ End UFactory Interface

	UPROPERTY(EditAnywhere, Category = ItemDefinition)
	TSubclassOf<UItemDefinition> ItemDefinitionClass;
};
