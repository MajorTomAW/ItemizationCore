// Author: Tom Werner (MajorT), 2025 November


#include "Toolkits/Tabs/ItemDefinitionAppTabFactory_DataList.h"

#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/IItemDefinitionApp.h"
#include "Toolkits/ItemDefinitionApp.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAppTabFactory_DataList"

FItemDefinitionAppTabFactory_DataList::FItemDefinitionAppTabFactory_DataList(
	const TSharedPtr<IItemDefinitionApp>& InApp)
		: FItemDefinitionAppTabFactory(UE::ItemizationEditor::Ids::TabId_DataList, InApp)
{
	TabLabel = LOCTEXT("TabLabel", "Data List");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get().GetStyleSetName(), "Icons.DataList");
	ViewMenuDescription = LOCTEXT("ViewDesc", "Data List");
	ViewMenuTooltip = LOCTEXT("ViewTooltip", "Show the data list tab of the item definition asset");
}

TSharedRef<SWidget> FItemDefinitionAppTabFactory_DataList::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp<FItemDefinitionApp>()->SpawnTab_DataList(FSpawnTabArgs(nullptr, TabIdentifier));
}

FText FItemDefinitionAppTabFactory_DataList::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("TabTooltip", "View and edit the data list of the item definition.");
}

#undef LOCTEXT_NAMESPACE
