// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_CallFunction.h"
#include "K2Node_FindItemComponent.generated.h"

UCLASS()
class UK2Node_FindItemComponent : public UK2Node_CallFunction
{
	GENERATED_BODY()

public:
	UK2Node_FindItemComponent();

	//~ Begin UEdGraphNode Interface
	/*virtual void PostReconstructNode() override;
	
	
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;*/
	//~ End UEdGraphNode Interface
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void PostReconstructNode() override;
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;

	UPROPERTY()
	FName ItemComponentName;

protected:
	//~ Begin UK2Node Interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;
	virtual void AllocateDefaultPins() override;
	//virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual bool IsNodePure() const override { return true; }
	//~ End UK2Node Interface

	/** Make sure the output Payload wildcard matches the input PayloadType */ 
	void RefreshOutputPayloadType() const;

	/** Get the item component type input pin */
	UEdGraphPin* GetItemCompDataTypePin() const;

	/** Get the item component data output pin */
	UEdGraphPin* GetItemCompDataPin() const;

	UScriptStruct* GetStructToFind() const;
};
