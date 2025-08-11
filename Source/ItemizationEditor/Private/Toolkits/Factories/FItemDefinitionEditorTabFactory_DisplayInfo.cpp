// Copyright © 2025 Playton. All Rights Reserved.


#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/IItemDefinitionApplication.h"
#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_DisplayInfo.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionEditorTabFactory_DisplayInfo"

FItemDefinitionEditorTabFactory_DisplayInfo::FItemDefinitionEditorTabFactory_DisplayInfo(
	const TSharedPtr<IItemDefinitionApplication>& InApp)
		: FItemDefinitionEditorTabFactory(UE::ItemizationEditor::Ids::TabId_DisplayInfo, InApp)
{
	TabLabel = LOCTEXT("DetailsTabLabel", "Display Info");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.DisplayInfo");
	ViewMenuDescription = LOCTEXT("DetailsTabDesc", "Display Info");
	ViewMenuTooltip = LOCTEXT("DetailsTabMenuTooltip", "Show the display info tab of the item.");
}

TSharedRef<SWidget> FItemDefinitionEditorTabFactory_DisplayInfo::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SDockTab);
}

FText FItemDefinitionEditorTabFactory_DisplayInfo::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DisplayInfoTabTooltip", "Display information about the item definition, such as its name, description, and other metadata.");
}

#undef LOCTEXT_NAMESPACE