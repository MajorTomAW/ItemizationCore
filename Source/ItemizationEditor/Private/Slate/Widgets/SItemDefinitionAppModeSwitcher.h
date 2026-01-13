// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


class IItemDefinitionApp;

class SItemDefinitionAppModeSwitcher : public SCompoundWidget
{
	using ThisClass = SItemDefinitionAppModeSwitcher;
public:
	SLATE_BEGIN_ARGS(SItemDefinitionAppModeSwitcher)
		: _ContentPadding(FMargin(2.f, 3.f))
		, _bUseDropdown(true)
		{
		}
		SLATE_ARGUMENT(FMargin, ContentPadding)
		SLATE_ARGUMENT(bool, bUseDropdown)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedPtr<IItemDefinitionApp>& InApp, FToolBarBuilder& InToolBarBuilder);
	virtual ~SItemDefinitionAppModeSwitcher() override;

private:
	TSharedRef<SWidget> GetModeSwitcherContent();
	bool HandleIsEnabled() const;
	void OnAppModeChanged(FName NewModeName) const;

	bool Execute_IsModeActive(FName ModeName) const;
	void Execute_SetMode(FName ModeName) const;
	bool CanExecute_SetMode(FName ModeName) const;
	bool CanShow_SetMode(FName ModeName) const;

private:
	/** Holds the mode switcher widget. */
	TSharedPtr<SComboButton> Modes_ComboButton;
	TSharedPtr<SHorizontalBox> Modes_HorizontalBox;

	/** Holds the icon of the current mode. */
	TSharedPtr<SImage> ActiveModeImage;

	/** Holds the display name of the current mode. */
	TSharedPtr<STextBlock> ActiveModeDisplayName;

	/** Holds the application that this widget is associated with. */
	TWeakPtr<IItemDefinitionApp> WeakApp;

	/** True, if this mode switcher use a dropdown combo button for all modes rather than a single button for each one. */
	bool bUseDropdown = true;
};
