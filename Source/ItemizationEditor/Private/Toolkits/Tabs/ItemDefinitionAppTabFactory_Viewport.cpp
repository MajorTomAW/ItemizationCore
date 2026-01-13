// Author: Tom Werner (MajorT), 2025 November


#include "Toolkits/Tabs/ItemDefinitionAppTabFactory_Viewport.h"

#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/IItemDefinitionApp.h"
#include "Toolkits/ItemDefinitionApp.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAppTabFactory_Viewport"

FItemDefinitionAppTabFactory_Viewport::FItemDefinitionAppTabFactory_Viewport(
const TSharedPtr<IItemDefinitionApp>& InApp)
	: FItemDefinitionAppTabFactory(UE::ItemizationEditor::Ids::TabId_Viewport, InApp)
{
	TabLabel = LOCTEXT("TabLabel", "Viewport");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get().GetStyleSetName(), "Icons.Viewport");
	ViewMenuDescription = LOCTEXT("ViewDesc", "Viewport");
	ViewMenuTooltip = LOCTEXT("ViewTooltip", "Preview your item");
}

TSharedRef<SWidget> FItemDefinitionAppTabFactory_Viewport::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp<FItemDefinitionApp>()->SpawnTab_Viewport(FSpawnTabArgs(nullptr, TabIdentifier));
}

FText FItemDefinitionAppTabFactory_Viewport::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("TabTooltip", "The viewport for previewing");
}

#undef LOCTEXT_NAMESPACE
