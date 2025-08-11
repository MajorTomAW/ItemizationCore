// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemComponentData.h"
#include "Engine/DataAsset.h"
#include "Enums/EItemDataQueryResult.h"

#include "ItemDefinitionBase.generated.h"

class UInventoryItemInstance;

UCLASS(Blueprintable)
class ITEMIZATIONCORE_API UItemDefinitionBase
	: public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UItemDefinitionBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UPrimaryDataAsset Interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//~ End UPrimaryDataAsset Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostSaveRoot(FObjectPostSaveRootContext ObjectSaveContext) override;
#endif
	//~ End UObject Interface

	/** Custom K2 Node for finding item data by struct type. */
	UFUNCTION(BlueprintCallable, CustomThunk, Category="Itemization|Item",
		meta=(DisplayName="Query Item Data", CustomStructureParam="Value", ExpandEnumAsExecs="Result", BlueprintInternalUseOnly="true"))
	void K2_QueryItemData(EItemDataQueryResult& Result, UScriptStruct* ItemDataType, int32& Value);
	DECLARE_FUNCTION(execK2_QueryItemData);

	/** Returns all item data for this item. */
	TArray<const FItemComponentData*> GetDataList() const;

	/** Returns the raw item data list. */
	TArray<const FItemComponentDataInstance*> GetDataListRaw() const
	{
		TArray<const FItemComponentDataInstance*> Result;
		for (const FItemComponentDataInstance& Instance : DataList)
		{
			Result.Add(&Instance);
		}

		return Result;
	}

	/** Returns the specific item data for the given struct type. */
	const FItemComponentData* GetItemData(const UScriptStruct* PropertyType) const;
	template <typename PropertyType>
	const PropertyType* GetItemData() const
	{
		static_assert(TIsDerivedFrom<PropertyType, FItemComponentData>::IsDerived, "PropertyType must be derived from FItemComponentData");
		return static_cast<const PropertyType*>(GetItemData(PropertyType::StaticStruct()));
	}

	TSoftClassPtr<UObject> GetItemInstanceClass() const { return ItemInstanceClass; }
	bool WantsItemInstance() const { return bWantsItemInstance; }

	/** Returns the item name as a plain text. */
	virtual FText GetItemName() const { return ItemName; }

	/** Returns the item name as a plain text. */
	virtual FText GetItemDescription(bool bFallbackToShort = true) const;

	/** Returns the item name as a rich text. */
	virtual FText GetItemRichDescription() const;

	/** Returns the item short description as a plain text. */
	virtual FText GetItemShortDescription() const { return ItemShortDescription; }

	/** Returns the documentation URL for this item. */
	FString GetDocumentationURL() const { return DocumentationURL; }

	virtual TArray<TSoftObjectPtr<const UScriptStruct>> GetDisallowedDataTypes() const;

protected:
	/** The friendly name to be shown in the UI for this item. */
	UPROPERTY(EditDefaultsOnly, Category=Item, AssetRegistrySearchable, DisplayName="Display Name")
	FText ItemName;

	/** The description of this item for summary information. */
	UPROPERTY(EditDefaultsOnly, Category=Item, DisplayName="Description")
	FText ItemDescription;

	/** The short description of this item for summary information. Mostly used for tooltips. */
	UPROPERTY(EditDefaultsOnly, Category=Item, DisplayName="Rich Description")
	FText ItemShortDescription;
	
	/** The URL to the documentation for this item. */ 
	UPROPERTY(EditDefaultsOnly, Category=General, meta = (DisplayName = "Documentation URL"))
	FString DocumentationURL;
	
	/** Whether this item wants an instance once given to an inventory. */
	UPROPERTY(EditDefaultsOnly, Category=General) 
	uint8 bWantsItemInstance:1;

	/** The class of the item instance that should be created when this item is given to an inventory. */
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition=bWantsItemInstance, MustImplement="/Script/ItemizationCore.InventoryItemInstanceInterface"), Category=General)
	TSoftClassPtr<UObject> ItemInstanceClass;


	
	/** The primary asset type that will be used for this item definition. */
	UPROPERTY(EditDefaultsOnly, Category=Asset, meta=(GetOptions="ItemizationCore.ItemizationCoreSettings.GetItemTypes", ShowSearchForItemCount=1), DisplayName="Item Type")
	FName ItemAssetType;

	/** The primary asset name that will be used for this item definition. */
	UPROPERTY(VisibleDefaultsOnly, Category=Asset)
	FName ItemAssetId;

	
	/** List of item components that are attached to this item. */
	UPROPERTY(EditDefaultsOnly, Category=Data, NoClear, meta=(ExcludeBaseStruct,ShowOnlyInnerProperties))
	TArray<FItemComponentDataInstance> DataList;
};
