// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InitialItemGrant.generated.h"

USTRUCT(BlueprintType)
struct FInitialItemGrant
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Item)
	TObjectPtr<class UItemDefinitionBase> Item;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Item, meta=(ClampMin=0, UIMin=0))
	int32 Count = 1;
};