// Author: Tom Werner (MajorT), 2025 November


#include "AssetDefinition_ItemDefinitionBase.h"

#include "ItemizationEditorModule.h"
#include "Items/ItemDefinitionBase.h"
#include "Styles/ItemizationEditorStyle.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_ItemDefinitionBase)

#define LOCTEXT_NAMESPACE "AssetDefinition_ItemDefinitionBase"

FText UAssetDefinition_ItemDefinitionBase::GetAssetDisplayName() const
{
	return LOCTEXT("DisplayName", "Item Definition");
}

FText UAssetDefinition_ItemDefinitionBase::GetAssetDisplayName(const FAssetData& AssetData) const
{
	return FText::FromName(AssetData.GetPrimaryAssetId().PrimaryAssetType);
}

FText UAssetDefinition_ItemDefinitionBase::GetAssetDescription(const FAssetData& AssetData) const
{
	if (const UItemDefinitionBase* ItemDefinition = Cast<UItemDefinitionBase>(AssetData.GetAsset()))
	{
		return ItemDefinition->GetItemDescription();
	}

	return FText::GetEmpty();
}

FLinearColor UAssetDefinition_ItemDefinitionBase::GetAssetColor() const
{
	return FItemizationEditorStyle::Get().GetColor("Colors.ItemDefinitionBase");
}

TSoftClassPtr<> UAssetDefinition_ItemDefinitionBase::GetAssetClass() const
{
	return UItemDefinitionBase::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_ItemDefinitionBase::GetAssetCategories() const
{
	static const auto Categories =
	{
		FAssetCategoryPath(FText::FromName("Items")),
		EAssetCategoryPaths::Basic,
	};

	return Categories;
}

UThumbnailInfo* UAssetDefinition_ItemDefinitionBase::LoadThumbnailInfo(const FAssetData& InAssetData) const
{
	return Super::LoadThumbnailInfo(InAssetData);
}

EAssetCommandResult UAssetDefinition_ItemDefinitionBase::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	IItemizationEditorModule& ItemizationEditor = IItemizationEditorModule::GetModule();
	for (UItemDefinitionBase* ItemDefinition : OpenArgs.LoadObjects<UItemDefinitionBase>())
	{
		ItemizationEditor.CreateItemDefinitionApp(EToolkitMode::Standalone, OpenArgs.ToolkitHost, ItemDefinition);
	}
	return EAssetCommandResult::Handled;
}

#undef LOCTEXT_NAMESPACE
