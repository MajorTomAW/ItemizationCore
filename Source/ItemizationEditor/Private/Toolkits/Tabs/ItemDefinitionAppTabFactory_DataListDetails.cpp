// Author: Tom Werner (MajorT), 2025 November


#include "Toolkits/Tabs/ItemDefinitionAppTabFactory_DataListDetails.h"

#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/IItemDefinitionApp.h"
#include "Toolkits/ItemDefinitionApp.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAppTabFactory_DataListDetails"

FItemDefinitionAppTabFactory_DataListDetails::FItemDefinitionAppTabFactory_DataListDetails(
	const TSharedPtr<IItemDefinitionApp>& InApp)
		: FItemDefinitionAppTabFactory(UE::ItemizationEditor::Ids::TabId_DataListDetails, InApp)
{
	TabLabel = LOCTEXT("TabLabel", "Details");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get().GetStyleSetName(), "Icons.Details");
	ViewMenuDescription = LOCTEXT("ViewDesc", "Details");
	ViewMenuTooltip = LOCTEXT("ViewTooltip", "Show the details tab of the selected item data");
}

TSharedRef<SWidget> FItemDefinitionAppTabFactory_DataListDetails::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp<FItemDefinitionApp>()->SpawnTab_DataListDetails(FSpawnTabArgs(nullptr, TabIdentifier));
}

FText FItemDefinitionAppTabFactory_DataListDetails::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("TabTooltip", "View and edit the details of the selected item data.");
}

#undef LOCTEXT_NAMESPACE
