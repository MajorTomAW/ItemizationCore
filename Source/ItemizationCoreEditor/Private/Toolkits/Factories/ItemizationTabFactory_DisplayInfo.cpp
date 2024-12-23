// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/Factories/ItemizationTabFactory_DisplayInfo.h"

#include "ItemizationCoreEditor.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemizationEditorApplication.h"

#define LOCTEXT_NAMESPACE "ItemizationEditorTabFactory"
using namespace UE::ItemizationCore::Editor;

FItemizationTabFactory_DisplayInfo::FItemizationTabFactory_DisplayInfo(TSharedPtr<FItemizationEditorApplication> InApp)
	: FItemizationEditorTabFactory(IDs::TabID_DisplayInfo(), InApp)
{
	TabLabel = LOCTEXT("TabLabel", "Display Info");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.DisplayInfo");
	ViewMenuDescription = LOCTEXT("ViewMenuDescription", "Display Info");
	ViewMenuTooltip = LOCTEXT("ViewMenuTooltip", "Show the Display Info tab");
}

TSharedRef<SWidget> FItemizationTabFactory_DisplayInfo::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp()->SpawnTab_DisplayInfo();
}

FText FItemizationTabFactory_DisplayInfo::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DisplayInfoTabTooltip", "Display Info");
}

#undef LOCTEXT_NAMESPACE