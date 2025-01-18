// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationAppMode_Components.h"

#include "ItemDefinition.h"
#include "Toolkits/ItemizationEditorApplication.h"
#include "Toolkits/Factories/ItemizationTabFactory_Components.h"

using namespace UE::ItemizationCore::Editor;


FItemizationAppMode_Components::FItemizationAppMode_Components(
	TSharedPtr<FItemizationEditorApplication> InApp, const IItemizationCoreEditorModule::FItemizationAppModeArgs& InArgs)
		: FItemizationEditorAppMode(InApp, InArgs)
{
	TabLayout = FTabManager::NewLayout("Standalone_TabLayout_ItemizationAppMode_Components_v0")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->AddTab(IDs::AppMode_Components(), ETabState::OpenedTab)
		)
	);

	TabSet.RegisterFactory(MakeShareable(new FItemizationTabFactory_Components(InApp)));

	InApp->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
}

void FItemizationAppMode_Components::PostActivateMode()
{
	const TSharedPtr<FItemizationEditorApplication> App = GetApp();
	UItemDefinition* Item = App->GetItemDefinition();
	App->GetComponentsView()->SetObject(Item, true);
}

bool FItemizationAppMode_Components::CanShowMode() const
{
	return FItemizationEditorAppMode::CanShowMode();
}
