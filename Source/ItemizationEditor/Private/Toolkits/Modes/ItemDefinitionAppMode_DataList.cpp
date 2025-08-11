// Author: Tom Werner (MajorT), 2025


#include "ItemDefinitionAppMode_DataList.h"

#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemDefinitionApplication.h"
#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_ComponentData.h"
#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_SelectionDetails.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAppMode_DataList"

const FName FItemDefinitionAppMode_DataList::ModeId = "ItemDefinitionAppMode_DataList";

FItemDefinitionAppMode_DataList::FItemDefinitionAppMode_DataList(const TSharedPtr<IItemDefinitionApplication>& InApp)
	: FItemDefinitionApplicationMode(UE::ItemizationEditor::FItemDefinitionAppModeInfo(
		ModeId,
		LOCTEXT("ModeLabel", "Data List"),
		LOCTEXT("ModeTooltip", "View and manage the data list of the item"),
		FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Components"),
		1), InApp)
{
	using namespace UE::ItemizationEditor;

	TabLayout = FTabManager::NewLayout("ItemDefinitionAppMode_DataList_Layout_v1.0.1")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewSplitter()
			->SetOrientation(Orient_Horizontal)

			// Left Side: Data List
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.4f)
				->AddTab(Ids::TabId_ComponentData, ETabState::OpenedTab)
			)

			// Right Side: Selection Details
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.6f)
				->AddTab(Ids::TabId_SelectionDetails, ETabState::OpenedTab)
			)
		)
	);

	TabSet.RegisterFactory(MakeShared<FItemDefinitionEditorTabFactory_ComponentData>(InApp));
	TabSet.RegisterFactory(MakeShared<FItemDefinitionEditorTabFactory_SelectionDetails>(InApp));

	AddModesToolbar(ToolbarExtender);
}

void FItemDefinitionAppMode_DataList::PostActivateMode()
{
	const TSharedPtr<FItemDefinitionApplication> AppPtr = GetAppAs<FItemDefinitionApplication>();
	AppPtr->GetSelectionDetailsView()->SetObject(nullptr, true);
}

#undef LOCTEXT_NAMESPACE
