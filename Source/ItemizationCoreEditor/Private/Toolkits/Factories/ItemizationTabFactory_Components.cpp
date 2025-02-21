// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/Factories/ItemizationTabFactory_Components.h"

#include "ItemizationCoreEditor.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemizationEditorApplication.h"

#define LOCTEXT_NAMESPACE "ItemizationEditor"
using namespace UE::ItemizationCore::Editor;

FItemizationTabFactory_Components::FItemizationTabFactory_Components(TSharedPtr<FItemizationEditorApplication> InApp)
	: FItemizationEditorTabFactory(IDs::AppMode_Components(), InApp)
{
	TabLabel = LOCTEXT("ComponentsTabLabel", "Components");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Components");
	ViewMenuDescription = LOCTEXT("ComponentsTabDesc", "Components");
	ViewMenuTooltip = LOCTEXT("ComponentsTabMenuTooltip", "Show the Components tab");
}

TSharedRef<SWidget> FItemizationTabFactory_Components::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetApp()->SpawnTab_Components();
}

FText FItemizationTabFactory_Components::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("ComponentsTabTooltip", "Components tab");
}

#undef LOCTEXT_NAMESPACE