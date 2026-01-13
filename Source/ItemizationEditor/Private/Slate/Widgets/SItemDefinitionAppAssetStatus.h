// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


class SLayeredImage;
class IItemDefinitionApp;

class SItemDefinitionAppAssetStatus : public SCompoundWidget
{
	using ThisClass = SItemDefinitionAppAssetStatus;
public:
	SLATE_BEGIN_ARGS(SItemDefinitionAppAssetStatus)
		{
		}

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedPtr<IItemDefinitionApp>& InApp);

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

	TWeakPtr<IItemDefinitionApp> WeakApp;

	TSharedPtr<SLayeredImage> IconWidget;
	TSharedPtr<STextBlock> TextBlock;
	FText SizeText;
};
