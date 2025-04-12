// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ItemAssetTypeId.generated.h"

/** An FName wrapper used to identify item asset types. */
USTRUCT(BlueprintType)
struct FItemAssetTypeId
{
	GENERATED_BODY()

	FItemAssetTypeId() = default;
	virtual ~FItemAssetTypeId() = default;
	FItemAssetTypeId(FName InAssetTypeId)
		: AssetTypeId(InAssetTypeId)
	{
	}

	bool IsValid() const { return AssetTypeId.IsValid(); }

public:
	/** The asset type id as an FName. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AssetTypeId, AssetRegistrySearchable)
	FName AssetTypeId = NAME_None;
};
