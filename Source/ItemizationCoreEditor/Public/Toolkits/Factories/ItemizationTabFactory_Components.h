// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/ItemizationEditorTabFactory.h"

class ITEMIZATIONCOREEDITOR_API FItemizationTabFactory_Components : public FItemizationEditorTabFactory
{
public:
	FItemizationTabFactory_Components(TSharedPtr<FItemizationEditorApplication> InApp);

protected:
	//~ Begin FWorkflowTabFactory Interface
	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;
	//~ End FWorkflowTabFactory Interface
};