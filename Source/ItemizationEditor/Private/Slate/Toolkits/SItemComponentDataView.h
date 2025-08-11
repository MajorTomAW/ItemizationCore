// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Items/Data/ItemComponentData.h"
#include "Widgets/SCompoundWidget.h"


class FItemComponentInstanceListEntry;
class FItemDefinitionViewModel;

class SItemComponentDataView : public SCompoundWidget
{
	using ThisClass = SItemComponentDataView;
	using Super = SCompoundWidget;
public:
	SLATE_BEGIN_ARGS(SItemComponentDataView)
		{
		}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, TSharedRef<FItemDefinitionViewModel> ItemDefinitionViewModel);

private:
	FReply HandleAddComponentDataClicked();



private:
	TSharedPtr<FItemDefinitionViewModel> ItemViewModel;
	
	TSharedPtr<SScrollBar> ExternalScrollbar;
	TSharedPtr<SScrollBox> ViewBox;
	
	TSharedPtr<SListView<TSharedPtr<FItemComponentInstanceListEntry> >> ComponentView;
	TArray<TSharedPtr<FItemComponentInstanceListEntry> > Components;

	TSharedRef<ITableRow> HandleGenerateComponentRow(TSharedPtr<FItemComponentInstanceListEntry>  InInstance, const TSharedRef<STableViewBase>& InOwnerTableView);
};
