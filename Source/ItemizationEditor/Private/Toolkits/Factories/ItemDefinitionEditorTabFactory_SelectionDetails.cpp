// Author: Tom Werner (MajorT), 2025


#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_SelectionDetails.h"

#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/IItemDefinitionApplication.h"
#include "Toolkits/IItemDefinitionEditorHost.h"
#include "Toolkits/ItemDefinitionApplication.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionEditorTabFactory_SelectionDetails"

FItemDefinitionEditorTabFactory_SelectionDetails::FItemDefinitionEditorTabFactory_SelectionDetails(
	const TSharedPtr<IItemDefinitionApplication>& InApp)
		: FItemDefinitionEditorTabFactory(UE::ItemizationEditor::Ids::TabId_SelectionDetails, InApp)
{
	TabLabel = LOCTEXT("SelectionDetailsTabLabel", "Selection Details");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Details");
	ViewMenuDescription = LOCTEXT("SelectionDetailsTabDesc", "Selection Details");
	ViewMenuTooltip = LOCTEXT("SelectionDetailsTabMenuTooltip", "Show the Selection Details tab of the item.");
}

TSharedRef<SWidget> FItemDefinitionEditorTabFactory_SelectionDetails::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetAppAs<FItemDefinitionApplication>()->SpawnTab_SelectionDetails(FSpawnTabArgs(nullptr, TabIdentifier));
}

FText FItemDefinitionEditorTabFactory_SelectionDetails::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("SelectionDetailsTabTooltip", "View and edit the Selection Details of the item definition.");
}

#undef LOCTEXT_NAMESPACE