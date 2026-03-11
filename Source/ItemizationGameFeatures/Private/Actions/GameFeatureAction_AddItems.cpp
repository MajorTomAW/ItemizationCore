// Author: Tom Werner (MajorT), 2025 November


#include "Actions/GameFeatureAction_AddItems.h"

#include "InventoryBase.h"
#include "Items/ItemAndCount.h"


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
		int32 Dummy;
		Inventory->GiveItem(ItemGrant.ItemDefinition, ItemGrant.StackSize, nullptr, FGameplayTag(), Dummy);
	}
}

void UGameFeatureAction_AddItems::OnClearItemizationData(
	AInventoryBase* Inventory,
	FObjectKey Object,
	const FGameFeatureStateChangeContext& ChangeContext)
{
}
