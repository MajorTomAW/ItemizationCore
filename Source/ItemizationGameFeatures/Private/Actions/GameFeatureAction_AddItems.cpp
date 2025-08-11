// Copyright © 2025 Playton. All Rights Reserved.


#include "Actions/GameFeatureAction_AddItems.h"

#include "Inventory/InventoryBase.h"
#include "Inventory/Operations/InventoryOp_ItemAction.h"
#include "Items/Data/InitialItemGrant.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddItems)

void UGameFeatureAction_AddItems::OnGameFeatureDeactivating(
	FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData))
	{
		
	}
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_AddItems::AddAdditionalAssetBundleData(
	FAssetBundleData& AssetBundleData)
{
	Super::AddAdditionalAssetBundleData(AssetBundleData);
}
#endif

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddItems::IsDataValid(
	FDataValidationContext& Context) const
{
	return Super::IsDataValid(Context);
}
#endif

void UGameFeatureAction_AddItems::OnInitItemizationData(
	AInventoryBase* Inventory,
	FObjectKey Object,
	const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* const World = Inventory->GetWorld();
	if (!World ||
		(World->WorldType == EWorldType::EditorPreview) ||
		(World->WorldType == EWorldType::GamePreview))
	{
		return;
	}

	for (const auto& ItemGrant : ItemsToGive)
	{
		FInventoryOp_ItemAction::FParams Params;
		Params.SourceInventory = Inventory;
		Params.TargetInventory = Inventory;
		Params.Delta = ItemGrant.Count;

		AInventoryBase::FCreateItemEntryParams CreateItemParams;
		CreateItemParams.ItemDefinition = ItemGrant.Item;
		CreateItemParams.SourceObject = Inventory;
		CreateItemParams.StackSize = ItemGrant.Count;

		Inventory->GiveItem(MoveTemp(Params), CreateItemParams);
	}
}

void UGameFeatureAction_AddItems::OnClearItemizationData(
	AInventoryBase* Inventory,
	FObjectKey Object,
	const FGameFeatureStateChangeContext& ChangeContext)
{
}
