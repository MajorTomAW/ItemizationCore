// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/Factories/ItemizationTabFactory_Equipment.h"

#include "ItemizationCoreEditor.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemizationEditorApplication.h"

#define LOCTEXT_NAMESPACE "ItemizationEditorTabFactory"
using namespace UE::ItemizationCore::Editor;

FItemizationTabFactory_Equipment::FItemizationTabFactory_Equipment(TSharedPtr<FItemizationEditorApplication> InApp)
	: FItemizationEditorTabFactory(IDs::TabID_Equipment(), InApp)
{
	TabLabel = LOCTEXT("EquipmentTabLabel", "Equipment");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Equipment");
	ViewMenuDescription = LOCTEXT("EquipmentTabLabel", "Equipment");
	ViewMenuTooltip = LOCTEXT("EquipmentTabTooltip", "Open the Equipment tab.");
}

TSharedRef<SWidget> FItemizationTabFactory_Equipment::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp()->SpawnTab_Equipment();
}

FText FItemizationTabFactory_Equipment::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("EquipmentTabTooltip", "The Equipment tab allows you to manage equipment data.");
}

#undef LOCTEXT_NAMESPACE
