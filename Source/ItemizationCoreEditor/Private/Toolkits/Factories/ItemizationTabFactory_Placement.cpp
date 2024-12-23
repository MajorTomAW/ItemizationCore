// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/Factories/ItemizationTabFactory_Placement.h"

#include "ItemizationCoreEditor.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemizationEditorApplication.h"

#define LOCTEXT_NAMESPACE "ItemizationEditorTabFactory"
using namespace UE::ItemizationCore::Editor;

FItemizationTabFactory_Placement::FItemizationTabFactory_Placement(TSharedPtr<FItemizationEditorApplication> InApp)
	: FItemizationEditorTabFactory(IDs::TabID_Placement(), InApp)
{
	TabLabel = LOCTEXT("PlacementTabLabel", "Placement");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Placement");
	ViewMenuDescription = LOCTEXT("PlacementTabLabel", "Placement");
	ViewMenuTooltip = LOCTEXT("PlacementTabLabel", "Open the Placement tab");
}

TSharedRef<SWidget> FItemizationTabFactory_Placement::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp()->SpawnTab_Placement();
}

FText FItemizationTabFactory_Placement::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("PlacementTabTooltip", "The Placement tab allows you to modify how this item will be placed in the world.");
}

#undef LOCTEXT_NAMESPACE