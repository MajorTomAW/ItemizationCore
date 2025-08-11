// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Framework/SlateDelegates.h"
#include "Input/Reply.h"
#include "Internationalization/Text.h"
#include "KismetPins/SGraphPinObject.h"
#include "Templates/SharedPointer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SWidget;
class UEdGraphPin;
class UScriptStruct;

class SItemComponentDataGraphPin : public SGraphPinObject
{
	using ThisClass = SItemComponentDataGraphPin;
public:
	SLATE_BEGIN_ARGS(SItemComponentDataGraphPin)
		{
		}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

protected:
	//~ Begin SGraphPinObject Interface
	virtual FReply OnClickUse() override;
	virtual bool AllowSelfPinWidget() const override;
	virtual TSharedRef<SWidget> GenerateAssetPicker() override;
	virtual FText GetDefaultComboText() const override;
	virtual FOnClicked GetOnUseButtonDelegate() override;
	//~ End SGraphPinObject Interface

private:
	void HandleOnPickedNewStruct(const UScriptStruct* ChosenStruct);
};
