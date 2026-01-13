// Author: Tom Werner (MajorT), 2025 November


#include "ItemDefinitionAppMode_Default.h"

#include "Items/ItemDefinitionBase.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemDefinitionApp.h"
#include "Toolkits/Tabs/ItemDefinitionAppTabFactory_Details.h"
#include "Toolkits/Tabs/ItemDefinitionAppTabFactory_Settings.h"
#include "Toolkits/Tabs/ItemDefinitionAppTabFactory_Viewport.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAppMode_Default"

const FName FItemDefinitionAppMode_Default::ModeId = "ItemDefinitionAppMode_Default";

FItemDefinitionAppMode_Default::FItemDefinitionAppMode_Default(
	const TSharedPtr<IItemDefinitionApp>& InApp)
		: FItemDefinitionAppMode(UE::ItemizationEditor::FItemDefinitionAppModeInfo(
			ModeId,
			LOCTEXT("ModeLabel", "Defaults"),
			LOCTEXT("ModeTooltip", "The default settings for the item."),
			FSlateIcon(FItemizationEditorStyle::Get().GetStyleSetName(), "Icons.Details"),
			INT_MIN),
			InApp)
{
	using namespace UE::ItemizationEditor;

	TabLayout = FTabManager::NewLayout("ItemDefinitionAppMode_Default_Layout_v1.3.2")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewSplitter()
			->SetOrientation(Orient_Horizontal)

			// Left Panel
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(.33f)

				// Viewport
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(.36f)
					->AddTab(Ids::TabId_Viewport, ETabState::OpenedTab)
				)

				// Display Info & Development
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(.64f)
					->AddTab(Ids::TabId_Settings, ETabState::OpenedTab)
					->SetForegroundTab(Ids::TabId_Settings)
				)
			)

			// Right Panel
			->SetSizeCoefficient(0.67f)

			// Details & Placement
			->Split
			(
				FTabManager::NewStack()
				->AddTab(Ids::TabId_AssetDetails, ETabState::OpenedTab)
				->SetForegroundTab(Ids::TabId_AssetDetails)
			)
		)
	);

	TabSet.RegisterFactory(MakeShared<FItemDefinitionAppTabFactory_Details>(InApp));
	TabSet.RegisterFactory(MakeShared<FItemDefinitionAppTabFactory_Settings>(InApp));
	TabSet.RegisterFactory(MakeShared<FItemDefinitionAppTabFactory_Viewport>(InApp));

	AddModesToolbar(ToolbarExtender);
}

void FItemDefinitionAppMode_Default::PostActivateMode()
{
	const TSharedPtr<FItemDefinitionApp> AppPtr = GetApp<FItemDefinitionApp>();
	UItemDefinitionBase* ItemDefinition = AppPtr.IsValid() ? AppPtr->GetItemDefinition() : nullptr;

	// If ItemDefinition is nullptr, that's also okay.
	AppPtr->GetAssetDetailsView()->SetObject(ItemDefinition, true);
	AppPtr->GetSettingsDetailsView()->SetObject(ItemDefinition, true);
}

#undef LOCTEXT_NAMESPACE
