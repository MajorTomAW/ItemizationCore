// Author: Tom Werner (MajorT), 2025 November

#pragma once
#include "GameplayTagContainer.h"

#include "InventoryGroupUIData.generated.h"

USTRUCT(BlueprintType)
struct FInventoryGroupUIData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag GroupTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
};

USTRUCT(BlueprintType)
struct FInventoryGroupUIDataList
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer AllowedInventoryTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FInventoryGroupUIData> GroupDataList;
};
