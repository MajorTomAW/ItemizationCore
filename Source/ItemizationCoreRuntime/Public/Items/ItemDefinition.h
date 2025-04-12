// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemAssetTypeId.h"
#include "ItemComponentData.h"
#include "Engine/DataAsset.h"
#include "StructUtils/InstancedStruct.h"
#include "ItemDefinition.generated.h"

/** Const data definition of a single item in the game. */
UCLASS(BlueprintType, Blueprintable, Config=Game, Const)
class ITEMIZATIONCORERUNTIME_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UPrimaryDataAsset Interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//~ End UPrimaryDataAsset Interface

public:
	/** An FName describing the type of this item. Should only be changed in very parent classes. */
	UPROPERTY(VisibleDefaultsOnly, Category=AssetManager)
	FItemAssetTypeId PrimaryAssetType;

	/**
	 * An FName describing the type of this item to the HUD. 
	 * This is used to group items in the inventory and should be unique for each item type.
	 * This is different from the PrimaryAssetType, which is used for the AssetManager.
	 */
	UPROPERTY(VisibleDefaultsOnly, Category=AssetManager, AssetRegistrySearchable)
	FItemAssetTypeId AssetType;

	/** */
	UPROPERTY(EditDefaultsOnly, Category=Item, NoClear, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FItemComponentData>> Components;
};
