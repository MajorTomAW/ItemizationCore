// Author: Tom Werner (MajorT), 2025 November


#include "SItemDefinitionAppModeSwitcher.h"

#include "SlateOptMacros.h"
#include "Toolkits/IItemDefinitionApp.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAppModeSwitcher"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemDefinitionAppModeSwitcher::Construct(
	const FArguments& InArgs,
	const TSharedPtr<IItemDefinitionApp>& InApp,
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
			return Lhs.Priority > Rhs.Priority;
		});

		for (const auto& Mode : SortedModes)
		{
			InToolBarBuilder.AddToolBarButton(
			FUIAction
				(
					FExecuteAction::CreateSP(this, &ThisClass::Execute_SetMode, Mode.Id),
					FCanExecuteAction::CreateSP(this, &ThisClass::CanExecute_SetMode, Mode.Id),
					FIsActionChecked::CreateSP(this, &ThisClass::Execute_IsModeActive, Mode.Id),
					FIsActionButtonVisible::CreateSP(this, &ThisClass::CanShow_SetMode, Mode.Id)
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

SItemDefinitionAppModeSwitcher::~SItemDefinitionAppModeSwitcher()
{
	if (const TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin())
	{
		AppPtr->OnAppModeChanged().RemoveAll(this);
	}
}

TSharedRef<SWidget> SItemDefinitionAppModeSwitcher::GetModeSwitcherContent()
{
	using namespace UE::ItemizationEditor;
	FMenuBuilder MenuBuilder = FMenuBuilder(true, nullptr);

	TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin();
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

bool SItemDefinitionAppModeSwitcher::HandleIsEnabled() const
{
	const TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin();
	if (!AppPtr)
	{
		return false;
	}

	// We need at least 2 different modes to enable the modes switcher
	return AppPtr->GetApplicationModes().Num() > 1;
}

void SItemDefinitionAppModeSwitcher::OnAppModeChanged(FName NewModeName) const
{
	if (const TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin())
	{
		if (TOptional<UE::ItemizationEditor::FItemDefinitionAppModeInfo> ModeInfo = AppPtr->GetModeInfo(NewModeName))
		{
			ActiveModeImage->SetImage(ModeInfo->Icon.GetIcon());
			ActiveModeDisplayName->SetText(ModeInfo->Label);
			ActiveModeDisplayName->SetToolTipText(ModeInfo->Tooltip);
		}
	}
}

bool SItemDefinitionAppModeSwitcher::Execute_IsModeActive(FName ModeName) const
{
	if (const TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin())
	{
		const FName CurrentMode = AppPtr->GetCurrentMode();
		return CurrentMode == ModeName;
	}


	return false;
}

void SItemDefinitionAppModeSwitcher::Execute_SetMode(FName ModeName) const
{
	if (const TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin())
	{
		AppPtr->SetCurrentMode(ModeName);
	}
}

bool SItemDefinitionAppModeSwitcher::CanExecute_SetMode(FName ModeName) const
{
	const TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin();
	if (!AppPtr)
	{
		return false;
	}

	// We need at least 2 different modes to enable the modes switcher
	return AppPtr->GetApplicationModes().Num() > 1;
}

bool SItemDefinitionAppModeSwitcher::CanShow_SetMode(FName ModeName) const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
