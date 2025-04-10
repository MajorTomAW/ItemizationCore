// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"

#include "InventorySetupDataFactory.generated.h"

UCLASS()
class UInventorySetupDataFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UInventorySetupDataFactory();
	
	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual FText GetToolTip() const override;
	virtual FText GetDisplayName() const override;
	//~ End UFactory Interface

private:
	UClass* SelectedClass;
};
