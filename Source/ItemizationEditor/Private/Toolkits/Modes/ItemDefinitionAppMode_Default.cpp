// Author: Tom Werner (MajorT), 2025


#include "ItemDefinitionAppMode_Default.h"

#include "Items/ItemDefinitionBase.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemDefinitionApplication.h"
#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_Details.h"
#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_Developer.h"
#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_DisplayInfo.h"
#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_Viewport.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAppMode_Default"

const FName FItemDefinitionAppMode_Default::ModeId = "ItemDefinitionAppMode_Default";

FItemDefinitionAppMode_Default::FItemDefinitionAppMode_Default(const TSharedPtr<IItemDefinitionApplication>& InApp)
	: FItemDefinitionApplicationMode(UE::ItemizationEditor::FItemDefinitionAppModeInfo(
			ModeId,
			LOCTEXT("ModeLabel", "Defaults"),
			LOCTEXT("ModeTooltip", "The default settings for the item."),
			FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Details"),
			INT_MIN
			),InApp)
{
	using namespace UE::ItemizationEditor;
	
	TabLayout = FTabManager::NewLayout("ItemDefinitionAppMode_Default_Layout_v1.2.0")
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
					->AddTab(Ids::TabId_DisplayInfo, ETabState::OpenedTab)
					->AddTab(Ids::TabId_Developer, ETabState::OpenedTab)
					->SetForegroundTab(Ids::TabId_DisplayInfo)
				)
			)

			// Right Panel
			->SetSizeCoefficient(0.67)

			// Details & Placement
			->Split
			(
				FTabManager::NewStack()
				->AddTab(Ids::TabId_Details, ETabState::OpenedTab)
				->SetForegroundTab(Ids::TabId_Details)
			)
		)
	);

	TabSet.RegisterFactory(MakeShared<FItemDefinitionEditorTabFactory_Details>(InApp));
	TabSet.RegisterFactory(MakeShared<FItemDefinitionEditorTabFactory_DisplayInfo>(InApp));
	TabSet.RegisterFactory(MakeShared<FItemDefinitionEditorTabFactory_Developer>(InApp));
	TabSet.RegisterFactory(MakeShared<FItemDefinitionEditorTabFactory_Viewport>(InApp));
	
	AddModesToolbar(ToolbarExtender);
}

void FItemDefinitionAppMode_Default::PostActivateMode()
{
	const TSharedPtr<FItemDefinitionApplication> AppPtr = GetAppAs<FItemDefinitionApplication>();
	UItemDefinitionBase* ItemDefinition = AppPtr.IsValid() ? AppPtr->GetItemDefinition() : nullptr;

	// If ItemDefinition is nullptr, that's also okay.
	AppPtr->GetAssetDetailsView()->SetObject(ItemDefinition, true);
}

#undef LOCTEXT_NAMESPACE
