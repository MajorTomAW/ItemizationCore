// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


class IItemDefinitionApp;

class SItemDefinitionAppAssetType : public SCompoundWidget
{
	using ThisClass = SItemDefinitionAppAssetType;
public:
	SLATE_BEGIN_ARGS(SItemDefinitionAppAssetType)
		{
		}

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedPtr<IItemDefinitionApp>& InApp);

private:
	FText HandleGetAssetTypeText() const;
	const FSlateBrush* HandleGetAssetTypeIcon() const;

	UClass* GetAssetClass() const;

private:
	TWeakPtr<IItemDefinitionApp> WeakApp;
};
