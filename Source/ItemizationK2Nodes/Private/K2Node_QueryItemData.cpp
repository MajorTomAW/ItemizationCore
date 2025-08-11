// Author: Tom Werner (MajorT), 2025


#include "K2Node_QueryItemData.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "Items/ItemDefinitionBase.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_QueryItemData)

namespace U2KNode_QueryItemData_Private
{
	static const FName ItemDataTypePinName(TEXT("ItemDataType"));
	static const FName ItemDataOutputPinName(TEXT("Value"));
}

UK2Node_QueryItemData::UK2Node_QueryItemData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bDefaultsToPureFunc = false;
}

void UK2Node_QueryItemData::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);

	UClass* Action = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(Action))
	{
		auto CustomizeLambda = [](UEdGraphNode* NewNode, bool bIsTemplateNode, const FName FunctionName)
		{
			UK2Node_QueryItemData* Node = CastChecked<UK2Node_QueryItemData>(NewNode);
			const UFunction* Function = UItemDefinitionBase::StaticClass()->FindFunctionByName(FunctionName);
			check(Function);
			Node->SetFromFunction(Function);
		};

		// Use our Custom Thunk function
		UBlueprintNodeSpawner* GetNodeSpawner = UBlueprintNodeSpawner::Create(Action);
		check(GetNodeSpawner);

		GetNodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(
			CustomizeLambda,
			GET_FUNCTION_NAME_CHECKED(UItemDefinitionBase, K2_QueryItemData));

		ActionRegistrar.AddBlueprintAction(Action, GetNodeSpawner);
	}
}

void UK2Node_QueryItemData::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);

	if (Pin->PinName == U2KNode_QueryItemData_Private::ItemDataTypePinName)
	{
		if (Pin->LinkedTo.IsEmpty())
		{
			RefreshOutputStructType();
		}
	}
}

void UK2Node_QueryItemData::PostReconstructNode()
{
	Super::PostReconstructNode();

	RefreshOutputStructType();
}

void UK2Node_QueryItemData::ClearCachedBlueprintData(UBlueprint* Blueprint)
{
	Super::ClearCachedBlueprintData(Blueprint);

	RefreshOutputStructType();
}

void UK2Node_QueryItemData::RefreshOutputStructType()
{
	// Grab the value pin (the return pin with our data
	UEdGraphPin* ValuePin = GetItemDataOutputPin();
 
	// Our type struct pin
	UEdGraphPin* StructTypePin =  GetItemDataTypePin();
 
	if (StructTypePin->DefaultObject != ValuePin->PinType.PinSubCategoryObject)
	{
		if (ValuePin->SubPins.Num() > 0)
		{
			// If the pin has been broken (split), recombine it.
			GetSchema()->RecombinePin(ValuePin);
		}
		
		// Set the value of our value pin to your selected InstancedStructType
		ValuePin->PinType.PinSubCategoryObject = StructTypePin->DefaultObject;
		ValuePin->PinType.PinCategory = (StructTypePin->DefaultObject == nullptr)
			? UEdGraphSchema_K2::PC_Wildcard
			: UEdGraphSchema_K2::PC_Struct;
	}
}

UEdGraphPin* UK2Node_QueryItemData::GetItemDataTypePin() const
{
	return FindPinChecked(U2KNode_QueryItemData_Private::ItemDataTypePinName, EGPD_Input);
}

UEdGraphPin* UK2Node_QueryItemData::GetItemDataOutputPin() const
{
	return FindPinChecked(U2KNode_QueryItemData_Private::ItemDataOutputPinName, EGPD_Output);
}