// Author: Tom Werner (MajorT), 2025

#pragma once

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"

#include "GameFeatureAction_Itemization.generated.h"

#define MY_API ITEMIZATIONGAMEFEATURES_API

class AInventoryBase;
class FDelegateHandle;
class UGameInstance;
class UObject;
struct FGameFeatureActivatingContext;
struct FGameFeatureDeactivatingContext;
struct FGameFeatureStateChangeContext;
struct FWorldContext;

/** Basic itemization game feature action. */
UCLASS(Abstract, MinimalAPI)
class UGameFeatureAction_Itemization : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction Interface
	MY_API virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	MY_API virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~ End UGameFeatureAction Interface

protected:
	/** Called when the game instance starts */
	MY_API void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);

	/** Called when the world got valid. */
	MY_API virtual void OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext);

	/** Called when the actor extension has changed. */
	MY_API virtual void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	/** Subclasses should override this to add their itemization-specific functionality */
	MY_API virtual void OnInitItemizationData(AInventoryBase* Inventory, FObjectKey Object, const FGameFeatureStateChangeContext& ChangeContext)
		PURE_VIRTUAL(UGameFeatureAction_Itemization::OnInitItemizationData, );

	/** Subclasses should override this to remove their itemization-specific functionality */
	MY_API virtual void OnClearItemizationData(AInventoryBase* Inventory, FObjectKey Object, const FGameFeatureStateChangeContext& ChangeContext)
		PURE_VIRTUAL(UGameFeatureAction_Itemization::OnClearItemizationData, );

private:
	struct FItemizationData
	{
		FDelegateHandle GameInstanceStartDelegate;
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
	};
	
	TMap<FGameFeatureStateChangeContext, FItemizationData> ItemizationData;
};

#undef MY_API