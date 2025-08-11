// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Factories/Factory.h"

#include "ItemDefinitionAssetFactory.generated.h"

class UItemDefinitionBase;

UCLASS(Config=Editor)
class UItemDefinitionAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UItemDefinitionAssetFactory();

protected:
	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual FText GetDisplayName() const override;
	virtual FString GetDefaultNewAssetName() const override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	//~ End UFactory Interface

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UItemDefinitionBase> SelectedItemClass;

	UPROPERTY(Config)
	TArray<TObjectPtr<UClass>> CommonItemClasses;
};
