// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


class IItemDefinitionApplication;
class UItemDefinitionBase;

class SItemDefinitionEditorAssetType : public SCompoundWidget
{
	using ThisClass = SItemDefinitionEditorAssetType;
public:
	SLATE_BEGIN_ARGS(SItemDefinitionEditorAssetType)
		{
		}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedPtr<IItemDefinitionApplication>& InApp);

private:
	FText HandleGetAssetTypeText() const;
	const FSlateBrush* HandleGetAssetTypeIcon() const;

	UClass* GetAssetClass() const;
	
private:
	TWeakPtr<IItemDefinitionApplication> WeakApp;
};
