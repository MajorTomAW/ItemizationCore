// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"

class FItemizationEditorApplication;

/**
 * Default tab factory for all Itemization Editor tabs
 */
class ITEMIZATIONCOREEDITOR_API FItemizationEditorTabFactory : public FWorkflowTabFactory
{
public:
	FItemizationEditorTabFactory(FName InTabId, TSharedPtr<FItemizationEditorApplication> InApp);

protected:
	TSharedPtr<FItemizationEditorApplication> GetApp() const { return WeakApp.Pin(); }

private:
	TWeakPtr<FItemizationEditorApplication> WeakApp;
};
