// Author: Tom Werner (MajorT), 2025 November


#include "Actions/GameFeatureAction_AddItems.h"

#include "Inventory/InventoryBase.h"
#include "Inventory/Operations/InventoryOp_GiveAction.h"
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
		FInventoryOp_GiveAction::FParams Params;
		Params.SourceInventory = Inventory;
		Params.TargetInventory = Inventory;
		Params.NumGive = ItemGrant.Count;

		AInventoryBase::FCreateItemEntryParams CreateItemParams(ItemGrant.Item, ItemGrant.Count, Inventory);

		Inventory->GiveItem(MoveTemp(Params), CreateItemParams);
	}
}

void UGameFeatureAction_AddItems::OnClearItemizationData(
	AInventoryBase* Inventory,
	FObjectKey Object,
	const FGameFeatureStateChangeContext& ChangeContext)
{
}
