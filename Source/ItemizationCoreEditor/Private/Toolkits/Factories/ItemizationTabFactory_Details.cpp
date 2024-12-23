// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/Factories/ItemizationTabFactory_Details.h"

#include "ItemizationCoreEditor.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemizationEditorApplication.h"

#define LOCTEXT_NAMESPACE "ItemizationEditorTabFactory"
using namespace UE::ItemizationCore::Editor;

FItemizationTabFactory_Details::FItemizationTabFactory_Details(TSharedPtr<FItemizationEditorApplication> InApp)
	: FItemizationEditorTabFactory(IDs::TabID_Details(), InApp)
{
	TabLabel = LOCTEXT("DetailsTabLabel", "Details");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Details");
	ViewMenuDescription = LOCTEXT("DetailsTabLabel", "Details");
	ViewMenuTooltip = LOCTEXT("DetailsTabTooltip", "Show the details tab of the item.");
}

TSharedRef<SWidget> FItemizationTabFactory_Details::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp()->SpawnTab_Details();
}

FText FItemizationTabFactory_Details::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DetailsTabTooltip", "Show the details tab of the item.");
}

#undef LOCTEXT_NAMESPACE
