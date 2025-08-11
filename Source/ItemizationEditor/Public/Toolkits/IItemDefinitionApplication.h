// Author: Tom Werner (MajorT), 2025

#pragma once

#include "WorkflowOrientedApp/WorkflowCentricApplication.h"

#define MY_API ITEMIZATIONEDITOR_API

class FItemDefinitionApplicationMode;
class UItemDefinitionBase;

namespace UE::ItemizationEditor
{
	struct FItemDefinitionAppModeInfo
	{
		FName Identifier;
		FText Label;
		FText Tooltip;
		FSlateIcon Icon;
		int32 Priority = 0;
	};
}

/** ItemDefinition Editor public interface */
class IItemDefinitionApplication
	: public FWorkflowCentricApplication
	, public FNotifyHook
{
public:
	/** Delegate to be called whenever the active mode changes. */
	DECLARE_EVENT_OneParam(ThisClass, FOnAppModeChanged, FName /*NewModeName*/);
	
	/** Gets the delegate to be called whenever the active mode changes. */
	virtual FOnAppModeChanged& OnAppModeChanged() = 0;

	/** Returns all available application mode names. */
	virtual TArray<FName> GetApplicationModes() const = 0;
	
	/** Gets the current object being edited as an ItemDefinitionBase. */
	MY_API virtual UItemDefinitionBase* GetItemDefinition() const = 0;
	
	/** Gets the information about a given mode. */
	MY_API virtual TOptional<UE::ItemizationEditor::FItemDefinitionAppModeInfo> GetModeInfo(FName ModeName) const = 0;
	
	/** Add an ItemDefinition application mode. */
	MY_API virtual void AddItemDefinitionAppMode(UE::ItemizationEditor::FItemDefinitionAppModeInfo ModeArgs, TSharedRef<FItemDefinitionApplicationMode> ApplicationMode) = 0;

	/** Removes an ItemDefinition application mode. */
	MY_API virtual void RemoveItemDefinitionAppMode(FName ModeName) = 0;
};


namespace UE::ItemizationEditor::Ids
{
	static const FName AppId = "ItemDefinitionEditor";
	
	static const FName TabId_Details = "ItemDefinitionEditor_TabId_Details";
	static const FName TabId_Developer = "ItemDefinitionEditor_TabId_Developer";
	static const FName TabId_DisplayInfo = "ItemDefinitionEditor_TabId_DisplayInfo";
	static const FName TabId_Viewport = "ItemDefinitionEditor_TabId_Viewport";
	static const FName TabId_ComponentData = "ItemDefinitionEditor_TabId_ComponentData";
	static const FName TabId_SelectionDetails = "ItemDefinitionEditor_TabId_SelectionDetails";

	static const FName LayoutLeftStackId = "ItemDefinitionEditor_LayoutLeftStackId";
	static const FName LayoutRightStackId = "ItemDefinitionEditor_LayoutRightStackId";
}

#undef MY_API