// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationAppMode_Default.h"

#include "ItemDefinition.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Toolkits/ItemizationEditorApplication.h"
#include "Toolkits/Factories/ItemizationTabFactory_Details.h"
#include "Toolkits/Factories/ItemizationTabFactory_Development.h"
#include "Toolkits/Factories/ItemizationTabFactory_DisplayInfo.h"
#include "Toolkits/Factories/ItemizationTabFactory_Placement.h"
#include "Toolkits/Factories/ItemizationTabFactory_Viewport.h"

using namespace UE::ItemizationCore::Editor;

FItemizationAppMode_Default::FItemizationAppMode_Default(
	TSharedPtr<FItemizationEditorApplication> InApp, const IItemizationCoreEditorModule::FItemizationAppModeArgs& InArgs)
		: FItemizationEditorAppMode(InApp, InArgs)
{
	TabLayout = FTabManager::NewLayout("Standalone_TabLayout_ItemizationAppMode_Default_v2")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewSplitter()
			->SetOrientation(Orient_Horizontal)

			// Left Panel
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(.33f)

				// Viewport
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(.36f)
					->AddTab(IDs::TabID_Viewport(), ETabState::OpenedTab)
				)

				// Display Info & Development
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(.64f)
					->AddTab(IDs::TabID_DisplayInfo(), ETabState::OpenedTab)
					->AddTab(IDs::TabID_Development(), ETabState::OpenedTab)
					->SetForegroundTab(IDs::TabID_DisplayInfo())
				)
			)

			// Right Panel
			->SetSizeCoefficient(0.67)

			// Details & Placement
			->Split
			(
				FTabManager::NewStack()
				->AddTab(IDs::TabID_Details(), ETabState::OpenedTab)
				->AddTab(IDs::TabID_Placement(), ETabState::OpenedTab)
				->SetForegroundTab(IDs::TabID_Details())
			)
		)
	);

	TabSet.RegisterFactory(MakeShareable(new FItemizationTabFactory_Viewport(InApp)));
	TabSet.RegisterFactory(MakeShareable(new FItemizationTabFactory_DisplayInfo(InApp)));
	TabSet.RegisterFactory(MakeShareable(new FItemizationTabFactory_Development(InApp)));
	TabSet.RegisterFactory(MakeShareable(new FItemizationTabFactory_Details(InApp)));
	TabSet.RegisterFactory(MakeShareable(new FItemizationTabFactory_Placement(InApp)));

	InApp->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
}

void FItemizationAppMode_Default::PostActivateMode()
{
	const TSharedPtr<FItemizationEditorApplication> App = GetApp();
	UItemDefinition* Item = App->GetItemDefinition();

	App->GetDetailsView()->SetObject(Item, true);
	App->GetDevelopmentView()->SetObject(Item, true);
	App->GetPlacementView()->SetObject(Item, true);
	App->GetDisplayInfoView()->SetObject(Item, true);
}
