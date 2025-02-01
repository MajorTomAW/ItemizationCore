// Copyright © 2024 Playton. All Rights Reserved.


#include "K2Node_FindItemComponent.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "ItemDefinition.h"
#include "KismetCompiler.h"
#include "Components/ItemComponentData.h"

#define LOCTEXT_NAMESPACE "K2Node"

namespace UK2Node_FindItemComponentHelper
{
	static FName ItemCompDataPinName(TEXT("ReturnValue"));
	static FName ItemCompDataTypePinName(TEXT("ComponentType"));
	static FName ItemDefPinName(TEXT("InItemDefinition"));
}

UK2Node_FindItemComponent::UK2Node_FindItemComponent()
{
	bDefaultsToPureFunc = true;
	UK2Node_CallFunction::SetFromFunction(UItemDefinition::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UItemDefinition, FindItemComponent)));
}

void UK2Node_FindItemComponent::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	if (ActionRegistrar.IsOpenForRegistration(GetClass()))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		
		ActionRegistrar.AddBlueprintAction(GetClass(), NodeSpawner);
	}
}

void UK2Node_FindItemComponent::PostReconstructNode()
{
	Super::PostReconstructNode();

	RefreshOutputPayloadType();
}

void UK2Node_FindItemComponent::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin == GetItemCompDataTypePin())
	{
		if (ChangedPin->LinkedTo.Num() == 0)
		{
			RefreshOutputPayloadType();
		}
	}
}

void UK2Node_FindItemComponent::GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const
{
	Super::GetPinHoverText(Pin, HoverTextOut);

	if (Pin.PinName == UK2Node_FindItemComponentHelper::ItemCompDataPinName)
	{
		HoverTextOut = HoverTextOut + LOCTEXT("ItemCompDataPinHoverText", "\n\nThe item component data that was found.").ToString();
	}
}

/*void UK2Node_FindItemComponent::PostReconstructNode()
{
	Super::PostReconstructNode();

	RefreshOutputPayloadType();
}



*/

void UK2Node_FindItemComponent::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	
	/*CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, FItemComponentData::StaticStruct(), UK2Node_FindItemComponentHelper::ItemCompDataPinName);

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UItemDefinition::StaticClass(), UK2Node_FindItemComponentHelper::ItemDefPinName);
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UScriptStruct::StaticClass(), UK2Node_FindItemComponentHelper::ItemCompDataTypePinName);*/
}


/*void UK2Node_FindItemComponent::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);
}

void UK2Node_FindItemComponent::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* NodeClass = GetClass();
	FName ItemCompName = FItemComponentData::StaticStruct()->GetFName();

	if (ActionRegistrar.IsOpenForRegistration(NodeClass))
	{
		auto RefreshClassActions = []()
		{
			FBlueprintActionDatabase::Get().RefreshClassActions(StaticClass());
		};

		auto CustomizeInputNodeLambda = [](UEdGraphNode* NewNode, bool bIsTemplateNode, FName ActionName)
		{
			UK2Node_FindItemComponent* InputNode = CastChecked<UK2Node_FindItemComponent>(NewNode);
			InputNode->ItemComponentName = ActionName;
		};

		static bool bRegisterOnce = true;
		if (bRegisterOnce)
		{
			bRegisterOnce = false;
			//FEditorDelegates::OnAssetsDeleted.Add
		}

		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeInputNodeLambda, ItemCompName);
		ActionRegistrar.AddBlueprintAction(NodeClass, NodeSpawner);
	}
}*/

FText UK2Node_FindItemComponent::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
	{
		return LOCTEXT("NodeTitle", "Find Item Component");
	}

	return LOCTEXT("NodeTitle", "Find Item Component");
}

FText UK2Node_FindItemComponent::GetTooltipText() const
{
	return Super::GetTooltipText();
}

FText UK2Node_FindItemComponent::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "Itemization Core");
}

void UK2Node_FindItemComponent::RefreshOutputPayloadType() const
{
	UEdGraphPin* ItemCompDataPin = GetItemCompDataPin();
	UEdGraphPin* ItemCompDataTypePin = GetItemCompDataTypePin();

	// See if the type has changed
	if (ItemCompDataTypePin->DefaultObject != ItemCompDataPin->PinType.PinSubCategoryObject)
	{
		// If we're changing the type, we need to break the link
		if (ItemCompDataPin->SubPins.Num() > 0)
		{
			GetSchema()->RecombinePin(ItemCompDataPin);
		}

		ItemCompDataPin->PinType.PinSubCategoryObject = ItemCompDataTypePin->DefaultObject;
		ItemCompDataPin->PinType.PinCategory = (ItemCompDataTypePin->DefaultObject == nullptr) ? UEdGraphSchema_K2::PC_Wildcard : UEdGraphSchema_K2::PC_Struct;
	}
}

UEdGraphPin* UK2Node_FindItemComponent::GetItemCompDataTypePin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_FindItemComponentHelper::ItemCompDataTypePinName);
	check(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_FindItemComponent::GetItemCompDataPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_FindItemComponentHelper::ItemCompDataPinName);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UScriptStruct* UK2Node_FindItemComponent::GetStructToFind() const
{
	UEdGraphPin* Pin = GetItemCompDataTypePin();
	return Cast<UScriptStruct>(Pin->PinType.PinSubCategoryObject.Get());
}

#undef LOCTEXT_NAMESPACE
