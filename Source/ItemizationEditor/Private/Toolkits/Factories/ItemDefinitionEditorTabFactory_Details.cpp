// Author: Tom Werner (MajorT), 2025


#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_Details.h"

#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/IItemDefinitionApplication.h"
#include "Toolkits/IItemDefinitionEditorHost.h"
#include "Toolkits/ItemDefinitionApplication.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionEditorTabFactory_Details"

FItemDefinitionEditorTabFactory_Details::FItemDefinitionEditorTabFactory_Details(
	const TSharedPtr<IItemDefinitionApplication>& InApp)
		: FItemDefinitionEditorTabFactory(UE::ItemizationEditor::Ids::TabId_Details, InApp)
{
	TabLabel = LOCTEXT("DetailsTabLabel", "Details");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Details");
	ViewMenuDescription = LOCTEXT("DetailsTabDesc", "Details");
	ViewMenuTooltip = LOCTEXT("DetailsTabMenuTooltip", "Show the details tab of the item.");
}

TSharedRef<SWidget> FItemDefinitionEditorTabFactory_Details::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetAppAs<FItemDefinitionApplication>()->SpawnTab_AssetDetails(FSpawnTabArgs(nullptr, TabIdentifier));
}

FText FItemDefinitionEditorTabFactory_Details::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DetailsTabTooltip", "View and edit the details of the item definition.");
}

#undef LOCTEXT_NAMESPACE
