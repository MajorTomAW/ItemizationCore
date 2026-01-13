// Author: Tom Werner (MajorT), 2025 November

#pragma once
#include "Items/Data/ItemComponentData.h"

class FItemDataUIInfo
{
public:
	FText LabelText;
	uint32 Index = 0;
	FItemComponentDataInstance* DataInstance = nullptr;
};
