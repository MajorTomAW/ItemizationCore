// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemAssetTypeId.h"
#include "ItemComponentData.h"
#include "Engine/DataAsset.h"
#include "Enums/EItemDataQueryResult.h"
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

	/** Returns the display of this item definition, if none is set, the class name will be used. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Item)
	FText GetDisplayName() const;

	/** Returns the raw item description. Will fall back to the short description if none is set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Item)
	FText GetItemDescription() const;

	/** Returns the raw short description. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Item)
	FText GetItemShortDescription() const;

	/** Returns the item description with rich text formatting. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Item)
	FText GetItemRichDescription() const;

	/** Tries to find the item component data for the given type. */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = Item, meta=(ExpandEnumAsExecs=ExecResult))
	void GetItemComponent(EItemDataQueryResult& ExecResult, UScriptStruct* PropertyType, FItemComponentData& FoundComponent) const;

	/** Returns the item component data as the given struct type. */
	const FItemComponentData* GetItemComponent(const UScriptStruct* PropertyType);
	template <typename T>
	FORCEINLINE const T* GetItemComponent() const
	{
		static_assert(TIsDerivedFrom<T, FItemComponentData>::IsDerived, "T must be derived from FItemComponentData");
		return GetItemComponent(T::StaticStruct());
	}

public:
	/** An FName describing the type of this item. Should only be changed in very parent classes. */
	UPROPERTY(VisibleDefaultsOnly, Category=AssetManager)
	FItemAssetTypeId PrimaryAssetType;

	/**
	 * An FName describing the type of this item to the HUD. 
	 * This is used to group items in the inventory and should be unique for each item type.
	 * This is different from the PrimaryAssetType, which is used for the AssetManager.
	 */
	UPROPERTY(VisibleDefaultsOnly, Category=AssetManager)
	FItemAssetTypeId AssetType;

#if WITH_EDITORONLY_DATA
	/** Editor-only registry searchable asset type as an FName. */
	UPROPERTY(AssetRegistrySearchable, DisplayName="Item Type")
	FName AssetType_Reg = NAME_None;
#endif
	//~ Begin UObject Interface
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;
#if WITH_EDITOR
	virtual void PostLoad() override;
	virtual void PostSaveRoot(FObjectPostSaveRootContext ObjectSaveContext) override;
#endif
	//~ End UObject Interface

protected:
	/** The friendly name to be shown in the UI for this item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, AssetRegistrySearchable, DisplayName="Display Name")
	FText ItemName;

	/** The description of this item for summary information. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, DisplayName = "Description")
	FText ItemDescription;

	/** The short description of this item for summary information. Mostly used for tooltips. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, DisplayName = "Short Description")
	FText ItemShortDescription;

	/** List of item components that are attached to this item. */
	UPROPERTY(EditDefaultsOnly, Category=Item, NoClear, meta=(ExcludeBaseStruct,ShowOnlyInnerProperties))
	TArray<TInstancedStruct<FItemComponentData>> ItemComponents;
};
