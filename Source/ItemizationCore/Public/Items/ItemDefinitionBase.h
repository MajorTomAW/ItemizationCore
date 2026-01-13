// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemComponentData.h"
#include "Engine/DataAsset.h"
#include "Enums/EItemDataQueryResult.h"

#if WINDOWS_USE_FEATURE_APPLICATIONMISC_CLASS
#include "Windows/WindowsPlatformApplicationMisc.h"
#endif

#include "ItemDefinitionBase.generated.h"

struct FGameplayTag;
class UInventoryItemInstance;

UCLASS(Blueprintable, PrioritizeCategories=("Item", "General"))
class ITEMIZATIONCORE_API UItemDefinitionBase
	: public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UItemDefinitionBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UPrimaryDataAsset Interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;
	//~ End UPrimaryDataAsset Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostSaveRoot(FObjectPostSaveRootContext ObjectSaveContext) override;
	virtual void PostRename(UObject* OldOuter, const FName OldName) override;
	virtual void PostLoad() override;
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

	/** Copies the asset id to the clipboard. */
	UFUNCTION(CallInEditor, Category="General")
	void CopyPrimaryAssetIdToClipboard() const
	{
#if WINDOWS_USE_FEATURE_APPLICATIONMISC_CLASS
		// Copy the FString to the clipboard
		FPlatformApplicationMisc::ClipboardCopy(*ItemId);
#endif
	}

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

	TArray<FItemComponentDataInstance*> GetDataListRaw_Mutable()
	{
		TArray<FItemComponentDataInstance*> Result;
		for (FItemComponentDataInstance& Instance : DataList)
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
	UFUNCTION(BlueprintCallable, Category=Item)
	virtual FText GetItemName(bool bUsePlural = false) const;

	/** Returns the item type name as a plain text. */
	UFUNCTION(BlueprintCallable, Category=Item)
	virtual FText GetItemTypeName() const;

	/** Returns the item description as a plain text. */
	UFUNCTION(BlueprintCallable, Category=Item)
	virtual FText GetItemDescription(bool bFallbackToShort = true) const;

	/** Returns the short item description as a rich text. */
	UFUNCTION(BlueprintCallable, Category=Item)
	virtual FText GetItemRichDescription() const;

	/** Returns the item short description as a plain text. */
	UFUNCTION(BlueprintCallable, Category=Item)
	virtual FText GetItemShortDescription() const { return ItemShortDescription; }

	/** Returns the documentation URL for this item. */
	UFUNCTION(BlueprintCallable, Category=Item)
	FString GetDocumentationURL() const { return DocumentationURL; }

	/** Returns true, if this item has the given trait tag. */
	UFUNCTION(BlueprintCallable, Category=Item, meta=(Categories="Item.Trait"))
	bool HasTrait(const FGameplayTag& TraitToCheck) const;

	virtual TArray<TSoftObjectPtr<const UScriptStruct>> GetDisallowedDataTypes() const;

protected:
	/** The friendly name to be shown in the UI for this item. */
	UPROPERTY(EditDefaultsOnly, Category=Item, DisplayName="Display Name", meta=(MultiLine))
	FText ItemName;

	/** The description of this item for summary information. */
	UPROPERTY(EditDefaultsOnly, Category=Item, DisplayName="Description", meta=(MultiLine))
	FText ItemDescription;

	/** The short description of this item for summary information. Mostly used for tooltips. */
	UPROPERTY(EditDefaultsOnly, Category=Item, DisplayName="Short Description", meta=(MultiLine))
	FText ItemShortDescription;

	/** The URL to the documentation for this item. */
	UPROPERTY(EditDefaultsOnly, Category=General, meta = (DisplayName = "Documentation URL"))
	FString DocumentationURL;

	/** Whether this item wants an instance once given to an inventory. */
	UPROPERTY(EditDefaultsOnly, Category=General)
	bool bWantsItemInstance;

	/** The class of the item instance that should be created when this item is given to an inventory. */
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition=bWantsItemInstance, MustImplement="/Script/ItemizationCore.InventoryItemInstanceInterface"), Category=General)
	TSoftClassPtr<UObject> ItemInstanceClass;



	/** The primary asset type that will be used for this item definition. */
	UPROPERTY(EditDefaultsOnly, Category=Asset, meta=(GetOptions="ItemizationCore.ItemizationCoreSettings.GetItemTypes", ShowSearchForItemCount=1), DisplayName="Item Type")
	FName ItemAssetType;

	/** The primary asset name that will be used for this item definition. */
	UPROPERTY(VisibleDefaultsOnly, Category=Asset)
	FName ItemAssetId;

	/** The full primary asset item id for external references. */
	UPROPERTY(VisibleDefaultsOnly, Category=Asset)
	FString ItemId;

public:
	/** List of item components that are attached to this item. */
	UPROPERTY(EditDefaultsOnly, Category=Data, NoClear, meta=(ExcludeBaseStruct,ShowOnlyInnerProperties))
	TArray<FItemComponentDataInstance> DataList;
};
