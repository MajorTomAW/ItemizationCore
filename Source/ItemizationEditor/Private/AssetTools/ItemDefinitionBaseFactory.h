// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "Factories/Factory.h"

#include "ItemDefinitionBaseFactory.generated.h"

class UItemDefinitionBase;

UCLASS(Config=Editor)
class UItemDefinitionBaseFactory : public UFactory
{
	GENERATED_BODY()

public:
	UItemDefinitionBaseFactory();

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
