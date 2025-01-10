// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/ItemizationEditorToolbar.h"

#include "Toolkits/ItemizationEditorApplication.h"
#include "Toolkits/ItemizationEditorAppMode.h"
#include "Widgets/SItemizationEditorAssetStatus.h"
#include "Widgets/SItemizationEditorAssetType.h"

#define LOCTEXT_NAMESPACE "ItemizationEditorToolbar"

void FItemizationEditorToolbar::AddModesToolbar(TSharedPtr<FExtender> Extender)
{
	TSharedPtr<FItemizationEditorApplication> App = GetApp();
	Extender->AddToolBarExtension
	(
		"Asset",
		EExtensionHook::After,
		App->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FItemizationEditorToolbar::FillModesToolbar)
	);
}

void FItemizationEditorToolbar::FillModesToolbar(FToolBarBuilder& ToolBarBuilder)
{
	TSharedPtr<FItemizationEditorApplication> App = GetApp();
	TArray<IItemizationCoreEditorModule::FItemizationAppModeArgs> ModesArgs;

	ToolBarBuilder.BeginSection("AssetStatus");
	ToolBarBuilder.AddWidget
	(
		SNew(SItemizationEditorAssetStatus, App)
	);
	ToolBarBuilder.EndSection();
	ToolBarBuilder.BeginSection("AssetInfo");
	ToolBarBuilder.AddWidget
	(
		SNew(SItemizationEditorAssetType)
		.ItemDefinition(App->GetItemDefinition())
	);
	ToolBarBuilder.EndSection();
	
	for (const auto& KVP : App->GetRegisteredApplicationModes())
	{
		const FItemizationEditorAppMode* ItemMode = static_cast<FItemizationEditorAppMode*>(KVP.Value.Get());
		if (ItemMode)
		{
			ModesArgs.Add(ItemMode->GetArgs());
		}
	}

	// No need to add mode buttons if there is just one mode
	if (ModesArgs.Num() <= 1)
	{
		return;
	}
	
	ModesArgs.Sort([](const IItemizationCoreEditorModule::FItemizationAppModeArgs& A, const IItemizationCoreEditorModule::FItemizationAppModeArgs& B)->bool
	{
		return A.Priority < B.Priority;
	});

	TMap<FName, TArray<IItemizationCoreEditorModule::FItemizationAppModeArgs>> ModesByHook;
	for (const auto& Mode : ModesArgs)
	{
		ModesByHook.FindOrAdd(Mode.ExtensionHookId).Add(Mode);
	}

	for (const auto& KVP : ModesByHook)
	{
		ToolBarBuilder.BeginSection(KVP.Key);

		for (const auto& Mode : KVP.Value)
		{
			ToolBarBuilder.AddToolBarButton
			(
				FUIAction
				(
					FExecuteAction::CreateSP(this, &FItemizationEditorToolbar::Execute_SetMode, Mode.ModeId),
					FCanExecuteAction::CreateSP(this, &FItemizationEditorToolbar::CanExecute_SetMode, Mode.ModeId),
					FIsActionChecked::CreateSP(this, &FItemizationEditorToolbar::Execute_IsModeActive, Mode.ModeId),
					FIsActionButtonVisible::CreateSP(this, &FItemizationEditorToolbar::CanShow_SetMode, Mode.ModeId)
				),
				NAME_None,
				Mode.ModeLabel,
				Mode.ModeTooltip,
				Mode.ModeIcon,
				EUserInterfaceActionType::ToggleButton
			);
		}

		ToolBarBuilder.EndSection();
	}
}

bool FItemizationEditorToolbar::Execute_IsModeActive(FName ModeName) const
{
	return GetApp()->GetCurrentMode() == ModeName;
}

bool FItemizationEditorToolbar::CanExecute_SetMode(FName ModeName) const
{
	return GetApp()->CanActivateMode(ModeName);
}

bool FItemizationEditorToolbar::CanShow_SetMode(FName ModeName) const
{
	return GetApp()->CanShowMode(ModeName);
}

void FItemizationEditorToolbar::Execute_SetMode(FName ModeName) const
{
	if (GetApp()->IsModeCurrent(ModeName))
	{
		return;
	}

	GetApp()->SetCurrentMode(ModeName);
}

#undef LOCTEXT_NAMESPACE