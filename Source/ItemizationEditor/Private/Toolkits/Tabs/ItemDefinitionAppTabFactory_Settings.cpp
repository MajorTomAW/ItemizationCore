// Author: Tom Werner (MajorT), 2025 November


#include "Toolkits/Tabs/ItemDefinitionAppTabFactory_Settings.h"

#include "ItemizationEditorHelpers.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/IItemDefinitionApp.h"
#include "Toolkits/ItemDefinitionApp.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAppTabFactory_Settings"

FItemDefinitionAppTabFactory_Settings::FItemDefinitionAppTabFactory_Settings(
	const TSharedPtr<IItemDefinitionApp>& InApp)
		: FItemDefinitionAppTabFactory(UE::ItemizationEditor::Ids::TabId_Settings, InApp)
{
	TabLabel = LOCTEXT("TabLabel", "Settings");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get().GetStyleSetName(), "Icons.Settings");
	ViewMenuDescription = LOCTEXT("ViewDesc", "Settings");
	ViewMenuTooltip = LOCTEXT("ViewTooltip", "Show the settings tab of the item definition asset");
}

TSharedRef<SWidget> FItemDefinitionAppTabFactory_Settings::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp<FItemDefinitionApp>()->SpawnTab_Settings(FSpawnTabArgs(nullptr, TabIdentifier));
}

FText FItemDefinitionAppTabFactory_Settings::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("TabTooltip", "View and edit the settings of the item definition.");
}

#undef LOCTEXT_NAMESPACE
