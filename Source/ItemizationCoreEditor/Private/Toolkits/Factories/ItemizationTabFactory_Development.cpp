// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/Factories/ItemizationTabFactory_Development.h"

#include "ItemizationCoreEditor.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemizationEditorApplication.h"

#define LOCTEXT_NAMESPACE "ItemizationEditor"
using namespace UE::ItemizationCore::Editor;

FItemizationTabFactory_Development::FItemizationTabFactory_Development(TSharedPtr<FItemizationEditorApplication> InApp)
	: FItemizationEditorTabFactory(IDs::TabID_Development(), InApp)
{
	TabLabel = LOCTEXT("DevelopmentTabLabel", "Settings");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Development");
	ViewMenuDescription = LOCTEXT("DevelopmentView", "Settings");
	ViewMenuTooltip = LOCTEXT("DevelopmentView_Tooltip", "Open the Development view");
}

TSharedRef<SWidget> FItemizationTabFactory_Development::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp()->SpawnTab_Development();
}

FText FItemizationTabFactory_Development::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DevelopmentTabTooltip", "The Settings tab allows you to view and edit the development settings for the item.");
}

#undef LOCTEXT_NAMESPACE