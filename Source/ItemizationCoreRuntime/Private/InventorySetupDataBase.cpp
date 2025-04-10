// Copyright © 2025 MajorT. All Rights Reserved.


#include "InventorySetupDataBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventorySetupDataBase)

UInventorySetupDataBase::UInventorySetupDataBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FInventoryPropertiesBase::FInventoryPropertiesBase()
	: TotalSlotsOverride(INDEX_NONE)
{
}

UInventorySetupDataBase_Default::UInventorySetupDataBase_Default(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}
