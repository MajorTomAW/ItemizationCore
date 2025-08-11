// Author: Tom Werner (MajorT), 2025


#include "ItemComponentDataPinFactory.h"

#include "K2Node_QueryItemData.h"
#include "SItemComponentDataGraphPin.h"

TSharedPtr<SGraphPin> FItemComponentDataPinFactory::CreatePin(UEdGraphPin* Pin) const
{
	// Make sure to filter out all pins that are not related to item data queries.
	if (Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Object)
	{
		return nullptr;
	}

	const UK2Node_QueryItemData* Node = Cast<UK2Node_QueryItemData>(Pin->GetOwningNode());
	if (Node == nullptr)
	{
		return nullptr;
	}

	if (Pin->PinName != "ItemDataType")
	{
		return nullptr;
	}

	// Now create our own pin widget.
	return SNew(SItemComponentDataGraphPin, Pin);
}
