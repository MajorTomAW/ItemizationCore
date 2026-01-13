// Author: Tom Werner (MajorT), 2025 November


#include "Toolkits/Tabs/ItemDefinitionAppTabFactory_Details.h"

#include "ItemizationEditorHelpers.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/IItemDefinitionApp.h"
#include "Toolkits/ItemDefinitionApp.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAppTabFactory_Details"

FItemDefinitionAppTabFactory_Details::FItemDefinitionAppTabFactory_Details(
	const TSharedPtr<IItemDefinitionApp>& InApp)
		: FItemDefinitionAppTabFactory(UE::ItemizationEditor::Ids::TabId_AssetDetails, InApp)
{
	TabLabel = LOCTEXT("TabLabel", "Details");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get().GetStyleSetName(), "Icons.Details");
	ViewMenuDescription = LOCTEXT("ViewDesc", "Details");
	ViewMenuTooltip = LOCTEXT("ViewTooltip", "Show the details tab of the item definition asset");
}

TSharedRef<SWidget> FItemDefinitionAppTabFactory_Details::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp<FItemDefinitionApp>()->SpawnTab_AssetDetails(FSpawnTabArgs(nullptr, TabIdentifier));
}

FText FItemDefinitionAppTabFactory_Details::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("TabTooltip", "View and edit the details of the item definition.");
}

#undef LOCTEXT_NAMESPACE
