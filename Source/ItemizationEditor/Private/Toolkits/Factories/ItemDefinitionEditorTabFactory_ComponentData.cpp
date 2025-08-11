// Author: Tom Werner (MajorT), 2025


#include "Toolkits/Factories/ItemDefinitionEditorTabFactory_ComponentData.h"

#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/IItemDefinitionApplication.h"
#include "Toolkits/ItemDefinitionApplication.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionEditorTabFactory_ComponentData"

FItemDefinitionEditorTabFactory_ComponentData::FItemDefinitionEditorTabFactory_ComponentData(
	const TSharedPtr<IItemDefinitionApplication>& InApp)
		: FItemDefinitionEditorTabFactory(UE::ItemizationEditor::Ids::TabId_ComponentData, InApp)
{
	TabLabel = LOCTEXT("ComponentDataTabLabel", "Data List");
	TabIcon = FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Components");
	ViewMenuDescription = LOCTEXT("ComponentDataViewMenu", "Data List");
	ViewMenuTooltip = LOCTEXT("ComponentDataViewTooltip", "Show the data list tab of the item definition.");
}

TSharedRef<SWidget> FItemDefinitionEditorTabFactory_ComponentData::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return GetAppAs<FItemDefinitionApplication>()->SpawnTab_ComponentData(FSpawnTabArgs(nullptr, TabIdentifier));
}

FText FItemDefinitionEditorTabFactory_ComponentData::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("ComponentDataTabTooltip", "View and manage the component data of the item definition, such as its properties and attributes.");
}

#undef LOCTEXT_NAMESPACE