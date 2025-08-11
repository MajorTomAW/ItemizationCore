// Author: Tom Werner (MajorT), 2025

#pragma once

#include "EdGraphUtilities.h"

class SGraphPin;
class UEdGraphPin;

struct FItemComponentDataPinFactory : public FGraphPanelPinFactory
{
public:
	virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* Pin) const override;
};