// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"

#include "GameFeatureAction_AddInventorySystem.generated.h"

struct FInventoryItemHandle;
class AInventoryBase;
struct FComponentRequestHandle;
class UInventorySystemConfiguration;

/** Data struct that describes a single inventory config entry. */
USTRUCT()
struct ITEMIZATIONCORERUNTIME_API FItemizationGameFeatureInventoryEntry
{
	GENERATED_BODY()

public:
	FItemizationGameFeatureInventoryEntry();

	/** The base actor class to add an inventory system to. */
	UPROPERTY(EditAnywhere, Category="Components", meta=(AllowAbstract="True"))
	TSoftClassPtr<AActor> ActorClass;

	/** The inventory configuration to use for this actor. */
	UPROPERTY(EditAnywhere, Category="Components")
	TObjectPtr<UInventorySystemConfiguration> InventoryConfig;

	/** Should this inventory system be added for clients? */
	UPROPERTY(EditAnywhere, Category="Components")
	uint8 bAddForClients : 1;

	/** Should this inventory system be added for the server? */
	UPROPERTY(EditAnywhere, Category="Components")
	uint8 bAddForServer : 1;
};

/**
 * Adds an inventory system request to the game
 * @warning This is an experimental class and will most likely change in the future
 */
UCLASS(DisplayName = "Add Inventory System")
class ITEMIZATIONCORERUNTIME_API UGameFeatureAction_AddInventorySystem : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~Begin UGameFeatureAction interface
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif
	//~End UGameFeatureAction interface

	//~Begin UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End UObject interface

	/** List of inventory system entries to add per actor when this game feature is enabled. */
	UPROPERTY(EditAnywhere, Category="Components", meta=(TitleProperty="{ActorClass} -> {InventoryConfig}"))
	TArray<FItemizationGameFeatureInventoryEntry> InventoryList;

private:
	struct FPerActorData
	{
		TArray<TWeakObjectPtr<AInventoryBase>> Inventories;
		TArray<FInventoryItemHandle> Items;
		TArray<TWeakObjectPtr<UActorComponent>> InventoryComponents;
	};

	struct FPerContextData
	{
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
		TMap<FObjectKey, FPerActorData> ActorData;

		FDelegateHandle GameInstanceStartHandle;
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

protected:
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext);

	void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);
	void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	void Reset(FPerContextData& ActiveData);
};
