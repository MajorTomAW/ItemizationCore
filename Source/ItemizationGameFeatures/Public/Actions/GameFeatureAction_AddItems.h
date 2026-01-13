// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction_Itemization.h"

#include "GameFeatureAction_AddItems.generated.h"

struct FInitialItemGrant;
struct FInventoryItemHandle;


/** GameFeatureAction responsible for adding items to an inventory once activated. */
UCLASS(MinimalAPI, DisplayName="Add Items")
class UGameFeatureAction_AddItems : public UGameFeatureAction_Itemization
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction Interface
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif
	//~ End UGameFeatureAction Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

protected:
	virtual void OnInitItemizationData(AInventoryBase* Inventory, FObjectKey Object, const FGameFeatureStateChangeContext& ChangeContext) override;
	virtual void OnClearItemizationData(AInventoryBase* Inventory, FObjectKey Object, const FGameFeatureStateChangeContext& ChangeContext) override;

private:
	/** List of items to give to the actor when the game feature is activated. */
	UPROPERTY(EditAnywhere, Category = "Items")
	TArray<FInitialItemGrant> ItemsToGive;

private:
	struct FPerActorData
	{
		TArray<FInventoryItemHandle> ItemsGiven;
	};

	struct FPerContextData
	{
		TMap<FObjectKey, FPerActorData> ActorData;
	};
	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;
};
