// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "InventoryEquipmentInstance.h"
#include "ItemizationCoreTypes.h"
#include "Components/EquipmentComponentData.h"
#include "Components/ItemComponentData.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

class UInventoryItemInstance;

/** Property metadata for the item definition. */
namespace UM
{
	enum
	{
		// [PropertyMetaData] Indicates for which item modes this item definition is allowed.
		AllowedItemModes,

		// [ClassMetaData] Indicates that properties that dont have the AllowedItemModes metadata won't be shown in the editor.
		ModesExclusive,
	};
}

/**
 * Minimal item definition asset which defines an item that can be given to a player's inventory.
 */
UCLASS(BlueprintType, Config = Game)
class ITEMIZATIONCORERUNTIME_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UPrimaryDataAsset Interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//~ End UPrimaryDataAsset Interface

	/**
	 * Returns the default instance class that will be used to create an instance of this item definition.
	 * Will fall back to UInventoryItemInstance if not set.
	 */
	TSubclassOf<UInventoryItemInstance> GetDefaultInstanceClass() const;

	/**
	 * Returns the default instance class that will be used to create an instance of this item/equipment definition.
	 * Will fall back to UInventoryEquipmentInstance if not set.
	 */
	TSubclassOf<UInventoryEquipmentInstance> GetDefaultEquipmentInstanceClass() const;

	/** Template method to get the specified instanced struct from the DisplayComponents array. */
	template<typename T>
	FORCEINLINE const T* GetDisplayComponent() const;
	TArray<const FItemComponentData*> GetDisplayComponents() const;

	/** Template method to get the specified instanced struct from the ItemComponents array. */
	template<typename T>
	FORCEINLINE const T* GetItemComponent() const;
	TArray<const FItemComponentData*> GetItemComponents() const;

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category = "Itemization Core")
	static FItemComponentData FindItemComponent(UItemDefinition* InItemDefinition, UScriptStruct* ComponentType)
	{
		for (const FItemComponentData* Comp : InItemDefinition->GetItemComponents())
		{
			if (Comp->StaticStruct() == ComponentType)
			{
				return *Comp;
			}
		}

		return FItemComponentData();
	}

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	//~ End UObject Interface
#endif

protected:
	/** The primary asset type that will be used for this item definition. */
	UPROPERTY(VisibleDefaultsOnly, AssetRegistrySearchable, Category = "Development|Asset Manager")
	FName PrimaryAssetType;

	/** The primary asset name that will be used for this item definition. */
	UPROPERTY(VisibleDefaultsOnly, AssetRegistrySearchable, Category = "Development|Asset Manager")
	FName PrimaryAssetName;

public:
	/** The friendly name to be shown in the UI for this item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", AssetRegistrySearchable, meta = (DisplayName = "Display Name"))
	FText ItemName;

	/** The description of this item for summary information. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (DisplayName = "Description"))
	FText ItemDescription;

	/** The short description of this item for summary information. Mostly used for tooltips. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (DisplayName = "Short Description"))
	FText ItemShortDescription;

	/** List of display components that will be used to modify the front-end display of this item. */
	UPROPERTY(EditDefaultsOnly, Category = "Display", NoClear, meta = (AllowCosmeticComponents = true, AllowNonCosmeticComponents = false))
	TArray<FItemComponentDataProxy> DisplayComponents;

	/** List of item components that will be used to modify the data/functionality of this item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", NoClear, meta = (AllowCosmeticComponents = false, AllowNonCosmeticComponents = true, AllowedItemModes = "AppMode_Components"))
	TArray<FItemComponentDataProxy> ItemComponents;

	UPROPERTY(EditDefaultsOnly, Category = "Equipment", NoClear, meta = (ExcludeBaseStruct = true))
	TInstancedStruct<FEquipmentComponentData> EquipmentData;

	/** The URL to the documentation for this item. */ 
	UPROPERTY(EditDefaultsOnly, Category = "Development", meta = (DisplayName = "Documentation URL"))
	FString DocumentationURL;
	
	/** The default instance class that will be used to create an instance of this item definition. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development", Config)
	TSoftClassPtr<UInventoryItemInstance> DefaultInstanceClass;

	/** The default instance class that will be used to create an equipment instance of this item definition. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Development", Config)
	TSoftClassPtr<UInventoryEquipmentInstance> DefaultEquipmentInstanceClass;
};

template <typename T>
FORCEINLINE const T* UItemDefinition::GetDisplayComponent() const
{
	for (const FItemComponentDataProxy& DisplayComponent : DisplayComponents)
	{
		if (DisplayComponent.Component.GetScriptStruct() == T::StaticStruct())
		{
			return &DisplayComponent.Component.Get<T>();
		}
	}

	return nullptr;
}

template <typename T>
FORCEINLINE const T* UItemDefinition::GetItemComponent() const
{
	for (const FItemComponentDataProxy& ItemComponent : ItemComponents)
	{
		if (ItemComponent.Component.GetScriptStruct() == T::StaticStruct())
		{
			return &ItemComponent.Component.Get<T>();
		}
	}

	return nullptr;
}
