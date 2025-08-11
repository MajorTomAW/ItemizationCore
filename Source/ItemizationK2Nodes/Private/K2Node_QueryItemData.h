// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "K2Node_CallFunction.h"
#include "K2Node_QueryItemData.generated.h"

UCLASS(MinimalAPI)
class UK2Node_QueryItemData : public UK2Node_CallFunction
{
	GENERATED_BODY()

public:
	UK2Node_QueryItemData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UEdGraphNode Interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PostReconstructNode() override;
	virtual void ClearCachedBlueprintData(UBlueprint* Blueprint) override;
	//~ End UEdGraphNode Interface

protected:
	void RefreshOutputStructType();
	
	/** Get the item data type input pin. */
	UEdGraphPin* GetItemDataTypePin() const;

	/** Get the item data output pin. */
	UEdGraphPin* GetItemDataOutputPin() const;

	/** Get the value output pin. */
	UEdGraphPin* GetValueOutputPin() const;
};
