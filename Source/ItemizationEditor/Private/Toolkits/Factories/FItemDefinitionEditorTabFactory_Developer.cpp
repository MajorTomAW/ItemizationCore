// Copyright © 2025 Playton. All Rights Reserved.


#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/IItemDefinitionApplication.h"
#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_Developer.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionEditorTabFactory_Developer"

FItemDefinitionEditorTabFactory_Developer::FItemDefinitionEditorTabFactory_Developer(
	const TSharedPtr<IItemDefinitionApplication>& InApp)
		: FItemDefinitionEditorTabFactory(UE::ItemizationEditor::Ids::TabId_Developer, InApp)
{
	TabLabel = LOCTEXT("DetailsTabLabel", "Settings");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Development");
	ViewMenuDescription = LOCTEXT("DetailsTabDesc", "Settings");
	ViewMenuTooltip = LOCTEXT("DetailsTabMenuTooltip", "Show the Settings tab of the item.");
}

TSharedRef<SWidget> FItemDefinitionEditorTabFactory_Developer::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SDockTab);
}

FText FItemDefinitionEditorTabFactory_Developer::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DisplayInfoTabTooltip", "Settings information about the item definition.");
}

#undef LOCTEXT_NAMESPACE