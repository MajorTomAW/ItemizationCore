// Copyright © 2025 Playton. All Rights Reserved.


#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/IItemDefinitionApplication.h"
#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_Viewport.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionEditorTabFactory_Viewport"

FItemDefinitionEditorTabFactory_Viewport::FItemDefinitionEditorTabFactory_Viewport(
	const TSharedPtr<IItemDefinitionApplication>& InApp)
		: FItemDefinitionEditorTabFactory(UE::ItemizationEditor::Ids::TabId_Viewport, InApp)
{
	TabLabel = LOCTEXT("DetailsTabLabel", "Viewport");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Viewport");
	ViewMenuDescription = LOCTEXT("DetailsTabDesc", "Viewport");
	ViewMenuTooltip = LOCTEXT("DetailsTabMenuTooltip", "Show the Viewport tab of the item.");
}

TSharedRef<SWidget> FItemDefinitionEditorTabFactory_Viewport::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SDockTab);
}

FText FItemDefinitionEditorTabFactory_Viewport::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DisplayInfoTabTooltip", "Developer information about the item definition, such as its name, description, and other metadata.");
}

#undef LOCTEXT_NAMESPACE