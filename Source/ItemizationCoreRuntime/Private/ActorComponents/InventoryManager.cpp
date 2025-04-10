// Copyright © 2025 MajorT. All Rights Reserved.


#include "ActorComponents/InventoryManager.h"

#include "Enums/EItemizationInventoryCreationType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryManager)

UInventoryManager::UInventoryManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RootInventory(nullptr)
	, CreationPolicy(EItemizationInventoryCreationType::Runtime)
{
}
