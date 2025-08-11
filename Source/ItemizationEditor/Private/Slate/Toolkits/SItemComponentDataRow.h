// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Items/Data/ItemComponentData.h"
#include "Widgets/SCompoundWidget.h"

class FItemComponentInstanceListEntry
{
public:
	FText DisplayName;
	const FItemComponentDataInstance* Instance = nullptr;
};

class SItemComponentDataRow : public STableRow<TWeakPtr<FItemComponentDataInstance>>
{
public:
	SLATE_BEGIN_ARGS(SItemComponentDataRow)
		{
		}

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, TSharedPtr<FItemComponentInstanceListEntry> InInstance, const TSharedPtr<SScrollBox>& ViewBox);
};
