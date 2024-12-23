// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/DataValidation.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Images/SLayeredImage.h"


class FItemizationEditorApplication;

class SItemizationEditorAssetStatus : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SItemizationEditorAssetStatus)
		{
		}

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, TSharedPtr<FItemizationEditorApplication> InApp);

private:
	const FSlateBrush* GetIconBrush() const;
	const FSlateBrush* GetOverlayIconBrush() const;
	FText GetStatusText() const;
	FText GetToolTipText() const;
	FReply OnClicked();
	
	void OnAssetSaved();
	void RefreshSize();

	EDataValidationResult AssetStatus = EDataValidationResult::Valid;
	bool bHasAnyWarnings = false;
	bool bManuallyValidated = false;
	
	
private:
	TSharedPtr<SLayeredImage> IconWidget;
	TSharedPtr<STextBlock> TextBlock;
	TWeakPtr<FItemizationEditorApplication> WeakApp;

	FText SizeText;
};
