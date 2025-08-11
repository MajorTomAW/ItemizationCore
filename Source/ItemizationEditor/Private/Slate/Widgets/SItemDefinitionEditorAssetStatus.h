// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Images/SLayeredImage.h"


class IItemDefinitionApplication;
class FItemDefinitionEdModeToolkit;

class SItemDefinitionEditorAssetStatus : public SCompoundWidget
{
	using ThisClass = SItemDefinitionEditorAssetStatus;
public:
	SLATE_BEGIN_ARGS(SItemDefinitionEditorAssetStatus)
		{
		}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedPtr<IItemDefinitionApplication>& InApp);

private:
	const FSlateBrush* GetIconBrush() const;
	const FSlateBrush* GetOverlayIconBrush() const;
	FText GetStatusText() const;
	FText GetToolTipText() const;
	FReply HandleOnClicked();

	void OnAssetSaved();
	void RefreshSize();

private:
	EDataValidationResult AssetStatus = EDataValidationResult::Valid;
	bool bHasAnyWarnings = false;
	bool bManuallyValidated = false;

	TWeakPtr<IItemDefinitionApplication> WeakApp;

	TSharedPtr<SLayeredImage> IconWidget;
	TSharedPtr<STextBlock> TextBlock;
	FText SizeText;
};
