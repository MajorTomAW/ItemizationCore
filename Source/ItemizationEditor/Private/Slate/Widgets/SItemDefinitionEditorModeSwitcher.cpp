// Copyright © 2025 Playton. All Rights Reserved.


#include "SItemDefinitionEditorModeSwitcher.h"

#include "SlateOptMacros.h"
#include "Toolkits/ItemDefinitionApplication.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionEditorModeSwitcher"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemDefinitionEditorModeSwitcher::Construct(
	const FArguments& InArgs,
	const TSharedPtr<IItemDefinitionApplication>& InApp,
	FToolBarBuilder& InToolBarBuilder)
{
	WeakApp = InApp;
	bUseDropdown = InArgs._bUseDropdown;

	if (bUseDropdown)
	{
		SAssignNew(Modes_ComboButton, SComboButton)
		.ContentPadding(InArgs._ContentPadding)
		.OnGetMenuContent(this, &ThisClass::GetModeSwitcherContent)
		.ButtonContent()
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(ActiveModeImage, SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
			]

			// Label
			+ SHorizontalBox::Slot()
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			.AutoWidth()
			[
				SAssignNew(ActiveModeDisplayName, STextBlock)
			]
		]
		.IsEnabled(this, &ThisClass::HandleIsEnabled);


		ChildSlot
		.Padding(FMargin(100.f, 2.f))
		[
			Modes_ComboButton.ToSharedRef()
		];




		InApp->OnAppModeChanged().AddSP(this, &ThisClass::OnAppModeChanged);
		if (TOptional<UE::ItemizationEditor::FItemDefinitionAppModeInfo> ModeInfo = InApp->GetModeInfo(InApp->GetCurrentMode()))
		{
			ActiveModeImage->SetImage(ModeInfo->Icon.GetIcon());
			ActiveModeDisplayName->SetText(ModeInfo->Label);
			ActiveModeDisplayName->SetToolTipText(ModeInfo->Tooltip);
		}
	}
	else
	{
		using namespace UE::ItemizationEditor;
		TArray<FItemDefinitionAppModeInfo> SortedModes;
		for (const FName& ModeId : InApp->GetApplicationModes())
		{
			if (TOptional<FItemDefinitionAppModeInfo> ModeInfo = InApp->GetModeInfo(ModeId); ModeInfo.IsSet())
			{
				SortedModes.Add(ModeInfo.GetValue());
			}
		}

		SortedModes.Sort([](const FItemDefinitionAppModeInfo& Lhs, const FItemDefinitionAppModeInfo& Rhs)
		{
			return Lhs.Priority < Rhs.Priority;
		});

		for (const auto& Mode : SortedModes)
		{
			InToolBarBuilder.AddToolBarButton(
			FUIAction
				(
					FExecuteAction::CreateSP(this, &ThisClass::Execute_SetMode, Mode.Identifier),
					FCanExecuteAction::CreateSP(this, &ThisClass::CanExecute_SetMode, Mode.Identifier),
					FIsActionChecked::CreateSP(this, &ThisClass::Execute_IsModeActive, Mode.Identifier),
					FIsActionButtonVisible::CreateSP(this, &ThisClass::CanShow_SetMode, Mode.Identifier)
				),
				NAME_None,
				Mode.Label,
				Mode.Tooltip,
				Mode.Icon,
				EUserInterfaceActionType::ToggleButton);
		}
	}
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

SItemDefinitionEditorModeSwitcher::~SItemDefinitionEditorModeSwitcher()
{
	if (const TSharedPtr<IItemDefinitionApplication> AppPtr = WeakApp.Pin())
	{
		AppPtr->OnAppModeChanged().RemoveAll(this);
	}
}

TSharedRef<SWidget> SItemDefinitionEditorModeSwitcher::GetModeSwitcherContent()
{
	using namespace UE::ItemizationEditor;
	FMenuBuilder MenuBuilder = FMenuBuilder(true, nullptr);

	TSharedPtr<IItemDefinitionApplication> AppPtr = WeakApp.Pin();
	const TArray<FName> Modes = AppPtr->GetApplicationModes();
	const FName CurrentMode = AppPtr->GetCurrentMode();

	MenuBuilder.BeginSection("Default Modes", LOCTEXT("LayoutsLabel", "Common Modes"));

	for (const FName& Mode : Modes)
	{
		// Only add modes that can be switched to
		if (Mode == CurrentMode)
		{
			continue;
		}

		if (TOptional<FItemDefinitionAppModeInfo> ModeInfo = AppPtr->GetModeInfo(Mode))
		{
			MenuBuilder.AddMenuEntry(
				ModeInfo->Label,
				ModeInfo->Tooltip,
				ModeInfo->Icon,
				FUIAction
				(
					FExecuteAction::CreateSP(this, &ThisClass::Execute_SetMode, Mode),
					FCanExecuteAction::CreateSP(this, &ThisClass::CanExecute_SetMode, Mode),
					FIsActionChecked::CreateSP(this, &ThisClass::Execute_IsModeActive, Mode),
					FIsActionButtonVisible::CreateSP(this, &ThisClass::CanShow_SetMode, Mode)
				),
				NAME_None,
				EUserInterfaceActionType::Button);
		}
	}

	MenuBuilder.EndSection();



	return MenuBuilder.MakeWidget();
}

bool SItemDefinitionEditorModeSwitcher::HandleIsEnabled() const
{
	const TSharedPtr<IItemDefinitionApplication> AppPtr = WeakApp.Pin();
	if (!AppPtr)
	{
		return false;
	}

	// We need at least 2 different modes to enable the modes switcher
	return AppPtr->GetApplicationModes().Num() > 0;
}

void SItemDefinitionEditorModeSwitcher::OnAppModeChanged(FName NewModeName) const
{
	if (const TSharedPtr<IItemDefinitionApplication> AppPtr = WeakApp.Pin())
	{
		if (TOptional<UE::ItemizationEditor::FItemDefinitionAppModeInfo> ModeInfo = AppPtr->GetModeInfo(NewModeName))
		{
			ActiveModeImage->SetImage(ModeInfo->Icon.GetIcon());
			ActiveModeDisplayName->SetText(ModeInfo->Label);
			ActiveModeDisplayName->SetToolTipText(ModeInfo->Tooltip);
		}
	}
}

bool SItemDefinitionEditorModeSwitcher::Execute_IsModeActive(FName ModeName) const
{
	if (const TSharedPtr<IItemDefinitionApplication> AppPtr = WeakApp.Pin())
	{
		const FName CurrentMode = AppPtr->GetCurrentMode();
		return CurrentMode == ModeName;
	}


	return false;
}

void SItemDefinitionEditorModeSwitcher::Execute_SetMode(FName ModeName) const
{
	if (const TSharedPtr<IItemDefinitionApplication> AppPtr = WeakApp.Pin())
	{
		AppPtr->SetCurrentMode(ModeName);
	}
}

bool SItemDefinitionEditorModeSwitcher::CanExecute_SetMode(FName ModeName) const
{
	return true;
}

bool SItemDefinitionEditorModeSwitcher::CanShow_SetMode(FName ModeName) const
{
	return true;
}
#undef LOCTEXT_NAMESPACE
