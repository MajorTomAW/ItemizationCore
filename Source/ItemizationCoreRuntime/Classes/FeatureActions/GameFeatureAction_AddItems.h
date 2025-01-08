// Copyright © 2024 Playton. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "InventoryItemEntryHandle.h"

#include "GameFeatureAction_WorldActionBase.h"
#include "Components/GameFrameworkComponentManager.h"

#include "GameFeatureAction_AddItems.generated.h"

class UItemDefinition;

USTRUCT()
struct FGameFeatureItemEntryGrant
{
	GENERATED_BODY()

public:
	/** The item definition to add. */
	UPROPERTY(EditAnywhere, Category = "Items")
	TSoftObjectPtr<UItemDefinition> ItemDefinition;

	/** The quantity of the item to add. */
	UPROPERTY(EditAnywhere, Category = "Items", meta = (ClampMin = "1", UIMin = "1"))
	int32 Quantity = 1;

	/** Whether the item wants to be equipped. */
	UPROPERTY(EditAnywhere, Category = "Items")
	bool bEquipItem = false;
};

/** Entry for a single item to add to an actor. */
USTRUCT()
struct FGameFeatureItemEntry
{
	GENERATED_BODY()

public:
	/** The base actor class to add to. */
	UPROPERTY(EditAnywhere, Category = "Items")
	TSoftClassPtr<AActor> ActorClass;
	

	/** List of items to grant to actors of the specified class. */
	UPROPERTY(EditAnywhere, Category = "Items", meta = (TitleProperty = "ItemDefinition", ShowOnlyInnerProperties))
	TArray<FGameFeatureItemEntryGrant> GrantedItems;
};

/**
 * GameFeatureAction responsible for adding items to actors in the world.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Items"))
class UGameFeatureAction_AddItems : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	UGameFeatureAction_AddItems();
	static const FName NAME_ItemAddReady;
	
	//~ Begin UGameFeatureAction interface
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~ End UGameFeatureAction interface

#if WITH_EDITOR
	//~ Begin UObject interface
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	//~ End UObject interface
#endif
	/** List of items to add to actors in the world. */
	UPROPERTY(EditAnywhere, Category = "Items", meta = (TitleProperty = "ActorClass", ShowOnlyInnerProperties))
	TArray<FGameFeatureItemEntry> ItemsList;

protected:
	//~ Begin UGameFeatureAction_WorldActionBase interface
	virtual void OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~ End UGameFeatureAction_WorldActionBase interface

private:
	struct FActorExtensions
	{
		TArray<FInventoryItemEntryHandle> Items;
	};

	struct FPerContextData
	{
		TMap<AActor*, FActorExtensions> ActiveExtensions;
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

protected:
	void Reset(FPerContextData& ActiveData);
	void HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext);
	void AddActorItems(AActor* Actor, const FGameFeatureItemEntry& ItemsEntry, FPerContextData& ActiveData);
	void RemoveActorItems(AActor* Actor, FPerContextData& ActiveData);

	template <class T>
	T* FindOrAddComponentForActor(AActor* Actor, const FGameFeatureItemEntry& ItemsEntry, FPerContextData& ActiveData)
	{
		return Cast<T>(FindOrAddComponentForActor(T::StaticClass(), Actor, ItemsEntry, ActiveData));
	}
	UActorComponent* FindOrAddComponentForActor(UClass* ComponentType, AActor* Actor, const FGameFeatureItemEntry& ItemsEntry, FPerContextData& ActiveData);
};