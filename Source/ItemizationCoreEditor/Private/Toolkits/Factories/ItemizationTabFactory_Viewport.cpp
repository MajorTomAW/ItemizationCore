// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/Factories/ItemizationTabFactory_Viewport.h"

#include "ItemizationCoreEditor.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemizationEditorApplication.h"

#define LOCTEXT_NAMESPACE "ItemizationEditor"
using namespace UE::ItemizationCore::Editor;

FItemizationTabFactory_Viewport::FItemizationTabFactory_Viewport(TSharedPtr<FItemizationEditorApplication> InApp)
	: FItemizationEditorTabFactory(IDs::TabID_Viewport(), InApp)
{
	TabLabel = LOCTEXT("ViewportTabLabel", "Viewport");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Viewport");
	ViewMenuDescription = LOCTEXT("ViewportDesc", "Viewport");
	ViewMenuTooltip = LOCTEXT("ViewportTabMenuTooltip", "Open the viewport tab");
}

TSharedRef<SWidget> FItemizationTabFactory_Viewport::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp()->SpawnTab_Viewport();
}

FText FItemizationTabFactory_Viewport::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("ViewportTabTooltip", "The viewport tab allows you to view the asset in a 3D environment.");
}

#undef LOCTEXT_NAMESPACE