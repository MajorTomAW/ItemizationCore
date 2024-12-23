// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/ItemizationEditorTabFactory.h"

#include "ItemizationCoreEditor.h"
#include "Toolkits/ItemizationEditorApplication.h"

#define LOCTEXT_NAMESPACE "ItemizationEditorTabFactory"
using namespace UE::ItemizationCore::Editor;

FItemizationEditorTabFactory::FItemizationEditorTabFactory(FName InTabId, TSharedPtr<FItemizationEditorApplication> InApp)
	: FWorkflowTabFactory(InTabId, InApp)
{
	bIsSingleton = true;
	WeakApp = InApp;
	TabRole = PanelTab;
}

#undef LOCTEXT_NAMESPACE