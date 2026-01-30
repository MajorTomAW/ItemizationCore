// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "ItemAndCount.generated.h"

class UItemDefinitionBase;

/** Wrapper struct for a single item definition and it's count. */
USTRUCT(BlueprintType)
struct FItemAndCount
{
	GENERATED_BODY()

	FItemAndCount()
		: ItemDefinition(nullptr)
		, StackSize(0)
	{
	}

	explicit FItemAndCount(const UItemDefinitionBase* InItemDefinition, int32 InCount = 1)
		: ItemDefinition(InItemDefinition)
		, StackSize(InCount)
	{
	}

public:
	/** The item definition. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=ItemAndCount)
	TObjectPtr<const UItemDefinitionBase> ItemDefinition;

	/** The count of the item. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=ItemAndCount)
	int32 StackSize;
};