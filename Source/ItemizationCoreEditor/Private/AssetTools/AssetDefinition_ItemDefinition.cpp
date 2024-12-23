// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetTools/AssetDefinition_ItemDefinition.h"

#include "ContentBrowserMenuContexts.h"
#include "ItemDefinition.h"
#include "ItemizationCoreEditor.h"
#include "ItemizationUtilities.h"
#include "Styles/ItemizationEditorStyle.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"

#define LOCTEXT_NAMESPACE "ItemizationCoreEditor"


namespace MenuExtension_ItemDefinition
{
	bool CanExecute_BrowseToSource(const FToolMenuContext& MenuContext)
	{
		return true;
	}

	void Execute_BrowserToSource(const FToolMenuContext& MenuContext)
	{
		
	}
	
	static FDelayedAutoRegisterHelper DelayerAutoRegister(EDelayedRegisterRunPhase::EndOfEngineInit, []
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
		{
			FToolMenuOwnerScoped OwnerScoped(UE_MODULE_NAME);
			UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UItemDefinition::StaticClass());

			FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
			
			Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateStatic([](FToolMenuSection& InSection)
			{
				{ // Browse to Source
					const TAttribute Label = LOCTEXT("ItemDefinition_BrowseToSource", "Browse to Source");
					const TAttribute ToolTip = LOCTEXT("ItemDefinition_BrowseToSource_ToolTip", "Browse to the source actor blueprint for this item definition.");
					const FSlateIcon Icon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Browse");

					FToolUIAction UIAction;
					UIAction.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&Execute_BrowserToSource);
					UIAction.IsActionVisibleDelegate = FToolMenuIsActionButtonVisible::CreateStatic(&CanExecute_BrowseToSource);

					InSection.AddMenuEntry("ItemDefinition_BrowseToSource", Label, ToolTip, Icon, UIAction);
				}

				{ // Make Public
					const TAttribute Label = LOCTEXT("ItemDefinition_MakePublic", "Make Public");
					const TAttribute ToolTip = LOCTEXT("ItemDefinition_MakePublic_ToolTip", "Make this item definition public.");
					const FSlateIcon Icon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Details");

					FToolUIAction UIAction;

					InSection.AddMenuEntry("ItemDefinition_MakePublic", Label, ToolTip, Icon, UIAction);
				}

				{ // Fix Up Asset ID
					const TAttribute Label = LOCTEXT("ItemDefinition_FixUpAssetId", "Fix Up Asset Id");
					const TAttribute ToolTip = LOCTEXT("ItemDefinition_FixUpAssetId_ToolTip", "Fix up the asset id for this item definition.");
					const FSlateIcon Icon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.FixUpAssetID");
					
					FToolUIAction UIAction;

					InSection.AddMenuEntry("ItemDefinition_FixUpAssetId", Label, ToolTip, Icon, UIAction);
				}

				{ // Find Referencers
					const TAttribute Label = LOCTEXT("ItemDefinition_FindReferencers", "Find Referencers");
					const TAttribute ToolTip = LOCTEXT("ItemDefinition_FindReferencers_ToolTip", "Find all references to this item definition.");
					const FSlateIcon Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.ReferenceViewer");

					FToolUIAction UIAction;

					InSection.AddMenuEntry("ItemDefinition_FindReferencers", Label, ToolTip, Icon, UIAction);
				}
			}));
		}));
	});
}

FText UAssetDefinition_ItemDefinition::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDisplayName", "Item Definition");
}

FText UAssetDefinition_ItemDefinition::GetAssetDescription(const FAssetData& AssetData) const
{
	if (const UItemDefinition* ItemDefinition = Cast<UItemDefinition>(AssetData.GetAsset()))
	{
		return UE::ItemizationCore::Display::GatherItemDesc_Any(ItemDefinition);
	}

	return FText::GetEmpty();
}

FLinearColor UAssetDefinition_ItemDefinition::GetAssetColor() const
{
	return FLinearColor(FItemizationEditorStyle::Get()->GetColor("Colors.ClassColor"));
}

TSoftClassPtr<UObject> UAssetDefinition_ItemDefinition::GetAssetClass() const
{
	return UItemDefinition::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_ItemDefinition::GetAssetCategories() const
{
	static const auto Categories =
	{
		FAssetCategoryPath(FText::FromName("Items")),
	};

	return Categories;
}

UThumbnailInfo* UAssetDefinition_ItemDefinition::LoadThumbnailInfo(const FAssetData& InAssetData) const
{
	UItemDefinition* ItemDefinition = Cast<UItemDefinition>(InAssetData.GetAsset());
	if (ItemDefinition == nullptr)
	{
		return nullptr;
	}

	return UE::Editor::FindOrCreateThumbnailInfo<USceneThumbnailInfo>(ItemDefinition);
}

EAssetCommandResult UAssetDefinition_ItemDefinition::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	IItemizationCoreEditorModule& ItemizationModule = IItemizationCoreEditorModule::Get();
	const EToolkitMode::Type Mode = OpenArgs.OpenMethod == EAssetOpenMethod::Edit ? EToolkitMode::Standalone : EToolkitMode::WorldCentric;

	for (UItemDefinition* Item : OpenArgs.LoadObjects<UItemDefinition>())
	{
		ItemizationModule.CreateItemizationApplication(Mode, OpenArgs.ToolkitHost, Item);
	}

	return EAssetCommandResult::Handled;
}

#undef LOCTEXT_NAMESPACE