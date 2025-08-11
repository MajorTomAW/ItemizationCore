// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "StructViewerModule.h"
#include "StructUtils/InstancedStruct.h"

#include "Widgets/SCompoundWidget.h"

inline auto GetCommonScriptStruct = [](const TSharedPtr<IPropertyHandle>& StructProperty, const UScriptStruct*& OutCommonStruct)
{
	bool bHasResult = false;
	bool bHasMultipleValues = false;
	
	StructProperty->EnumerateConstRawData([&OutCommonStruct, &bHasResult, &bHasMultipleValues](const void* RawData, const int32 /*DataIndex*/, const int32 /*NumDatas*/)
	{
		if (const FInstancedStruct* InstancedStruct = static_cast<const FInstancedStruct*>(RawData))
		{
			const UScriptStruct* Struct = InstancedStruct->GetScriptStruct();

			if (!bHasResult)
			{
				OutCommonStruct = Struct;
			}
			else if (OutCommonStruct != Struct)
			{
				bHasMultipleValues = true;
			}

			bHasResult = true;
		}

		return true;
	});

	if (bHasMultipleValues)
	{
		return FPropertyAccess::MultipleValues;
	}
	
	return bHasResult ? FPropertyAccess::Success : FPropertyAccess::Fail;
};


class SItemComponentDataPicker : public SCompoundWidget
{
	using Super = SCompoundWidget;
	using ThisClass = SItemComponentDataPicker;
public:
	SLATE_BEGIN_ARGS(SItemComponentDataPicker)
		{
		}
		SLATE_ARGUMENT(FOnStructPicked, OnStructPicked)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, TSharedPtr<IPropertyHandle> InStructProperty, TSharedPtr<IPropertyHandle> InStructInstanceProperty, TSharedPtr<IPropertyUtilities> InPropertyUtils);

	FOnStructPicked OnStructPicked;

protected:
	TSharedRef<SWidget> HandleGenerateStructPicker();
	void HandleStructPicked(const UScriptStruct* InStruct);
	
	const FSlateBrush* GetDisplayValueIcon() const;
	FText GetDisplayValueText() const;
	FText GetDisplayValueToolTipText() const;

protected:
	TSharedPtr<SComboButton> ComboButton;
	TSharedPtr<IPropertyHandle> StructProperty;
	TSharedPtr<IPropertyHandle> InstanceProperty;
	TSharedPtr<IPropertyUtilities> PropUtils;

	/** The base struct that we're allowing to be picked (controlled by the "BaseStruct" meta-data) */
	TWeakObjectPtr<UScriptStruct> BaseScriptStruct = nullptr;
};
