// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UItemDefinition;

class SItemizationEditorAssetType : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SItemizationEditorAssetType)
		{
		}
		SLATE_ARGUMENT(UItemDefinition*, ItemDefinition)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	FText HandleGetAssetTypeText() const;
	const FSlateBrush* HandleGetAssetIcon() const;
	UClass* GetAssetClass() const;

private:
	TWeakObjectPtr<UItemDefinition> ItemDefinition;
};
