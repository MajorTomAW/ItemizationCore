// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationAppMode_Equipment.h"

#include "ItemDefinition.h"
#include "Components/ItemComponentData_Equipment.h"
#include "Toolkits/ItemizationEditorApplication.h"
#include "Toolkits/Factories/ItemizationTabFactory_Development.h"
#include "Toolkits/Factories/ItemizationTabFactory_Equipment.h"
#include "Toolkits/Factories/ItemizationTabFactory_Viewport.h"

using namespace UE::ItemizationCore::Editor;

FItemizationAppMode_Equipment::FItemizationAppMode_Equipment(
	TSharedPtr<FItemizationEditorApplication> InApp, const IItemizationCoreEditorModule::FItemizationAppModeArgs& InArgs)
	: FItemizationEditorAppMode(InApp, InArgs)
{
	TabLayout = FTabManager::NewLayout("Standalone_TabLayout_ItemizationAppMode_Equipment_v1")
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

					// Development
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(.64f)
						->AddTab(IDs::TabID_Development(), ETabState::OpenedTab)
					)
				)

				// Right Panel
				->SetSizeCoefficient(.67f)

				// Equipment
				->Split
				(
					FTabManager::NewStack()
					->AddTab(IDs::TabID_Equipment(), ETabState::OpenedTab)
				)
			)
		);

	TabSet.RegisterFactory(MakeShareable(new FItemizationTabFactory_Viewport(InApp)));
	TabSet.RegisterFactory(MakeShareable(new FItemizationTabFactory_Development(InApp)));
	TabSet.RegisterFactory(MakeShareable(new FItemizationTabFactory_Equipment(InApp)));


	InApp->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
}

bool FItemizationAppMode_Equipment::CanActivateMode() const
{
	const UItemDefinition* ItemDefinition = GetApp() ? GetApp()->GetItemDefinition() : nullptr;
	if (ItemDefinition == nullptr)
	{
		return false;
	}

	if (const FItemComponentData_Equipment* Data_Equipment = ItemDefinition->GetItemComponent<FItemComponentData_Equipment>())
	{
		return Data_Equipment->bCanBeEquipped;
	}

	return false;
}

bool FItemizationAppMode_Equipment::CanShowMode() const
{
	const UItemDefinition* ItemDefinition = GetApp() ? GetApp()->GetItemDefinition() : nullptr;
	if (ItemDefinition == nullptr)
	{
		return false;
	}

	if (ItemDefinition->GetItemComponent<FItemComponentData_Equipment>())
	{
		return true;
	}

	return false;
}

void FItemizationAppMode_Equipment::PostActivateMode()
{
	const TSharedPtr<FItemizationEditorApplication> App = GetApp();
	UItemDefinition* Item = App->GetItemDefinition();

	App->GetDevelopmentView()->SetObject(Item, true);
	App->GetEquipmentView()->SetObject(Item, true);
}
