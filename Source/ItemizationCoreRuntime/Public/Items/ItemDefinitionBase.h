// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StructUtils/InstancedStruct.h"

#include "ItemDefinitionBase.generated.h"

class UInventoryItemInstance;
struct FItemComponentData;
struct FItemComponentDataInstance;
struct FInstancedStruct;

/**
 * 
 */
UCLASS(Blueprintable)
class ITEMIZATIONCORERUNTIME_API UItemDefinitionBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UItemDefinitionBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly)
	FText ItemName;

	UPROPERTY(EditDefaultsOnly)
	FText ItemDescription;

	UPROPERTY(EditDefaultsOnly)
	FText ItemShortDescription;

	UPROPERTY(EditDefaultsOnly)
	uint8 bWantsItemInstance:1;

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition=bWantsItemInstance))
	TSoftClassPtr<UInventoryItemInstance> ItemInstanceClass;

	/** Returns all item data for this item. */
	TArray<const FItemComponentData*> GetDataList() const;

	/** Returns the specific item data for the given struct type. */
	const FItemComponentData* GetItemData(const UScriptStruct* PropertyType) const;
	template <typename PropertyType>
	const PropertyType* GetItemData() const
	{
		static_assert(TIsDerivedFrom<PropertyType, FItemComponentData>::IsDerived, "PropertyType must be derived from FItemComponentData");
		return static_cast<const PropertyType*>(GetItemData(PropertyType::StaticStruct()));
	}

protected:
	/** List of item components that are attached to this item. */
	UPROPERTY(EditDefaultsOnly, Category=Item, NoClear, meta=(ExcludeBaseStruct,ShowOnlyInnerProperties))
	TArray<FItemComponentDataInstance> DataList;
};
