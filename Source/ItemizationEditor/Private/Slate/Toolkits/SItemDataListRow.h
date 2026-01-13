// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/Slate/ItemDataUIInfo.h"
#include "Widgets/SCompoundWidget.h"


class FItemDefinitionViewModel;

class SItemDataListRow : public STableRow<TSharedPtr<FItemDataUIInfo>>
{
	using ThisClass = SItemDataListRow;
public:
	SLATE_BEGIN_ARGS(SItemDataListRow)
		{
		}

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, TSharedPtr<FItemDataUIInfo> InItemData, const TSharedPtr<SScrollBox>& InViewBox, TSharedPtr<FItemDefinitionViewModel> InItemViewModel);

protected:
	const FSlateBrush* GetComponentIcon() const;
	FText GetComponentDisplayText() const;
	FText GetComponentDesc() const;
	FSlateColor GetSelectedColor() const;

private:
	TSharedPtr<FItemDataUIInfo> ItemData;
	TSharedPtr<FItemDefinitionViewModel> ViewModel;
};
