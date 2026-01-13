// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "WorkflowOrientedApp/WorkflowCentricApplication.h"

#define UE_API ITEMIZATIONEDITOR_API

class UItemDefinitionBase;
class FItemDefinitionAppMode;

namespace UE::ItemizationEditor
{
	struct FItemDefinitionAppModeInfo
	{
		FName Id;
		FText Label;
		FText Tooltip;
		FSlateIcon Icon;
		uint32 Priority = 0;
	};

	namespace Ids
	{
		static const FName AppId = "ItemDefinitionApp";

		static const FName TabId_AssetDetails = "ItemDefinitionApp_TabId_AssetDetails";
		static const FName TabId_DataList = "ItemDefinitionApp_TabId_DataList";
		static const FName TabId_DataListDetails = "ItemDefinitionApp_TabId_DataListDetails";
		static const FName TabId_Settings = "ItemDefinitionApp_TabId_Settings";
		static const FName TabId_Viewport = "ItemDefinitionApp_TabId_Viewport";

		static const FName LayoutLeftStackId = "ItemDefinitionApp_LayoutLeftStack";
		static const FName LayoutRightStackId = "ItemDefinitionApp_LayoutRightStack";
	}
}

/** ItemDefinition App public interface. */
class IItemDefinitionApp
	: public FWorkflowCentricApplication
{
public:
	/** Delegate to be called whenever the active mode changes. */
	DECLARE_EVENT_OneParam(IItemDefinitionApp, FOnAppModeChanged, FName /*NewModeName*/)

	/* Gets the delegate to be called whenever the active mode changes. */
	virtual FOnAppModeChanged& OnAppModeChanged() = 0;

	/** Returns all available application mode names. */
	virtual TArray<FName> GetApplicationModes() const = 0;

	/** Gets the current object being edited as an ItemDefinitionBase. */
	UE_API virtual UItemDefinitionBase* GetItemDefinition() const = 0;

	/** Gets the information about a given mode. */
	UE_API virtual TOptional<UE::ItemizationEditor::FItemDefinitionAppModeInfo> GetModeInfo(FName ModeName) const = 0;

	/** Add an ItemDefinition application mode. */
	UE_API virtual void AddItemDefinitionAppMode(UE::ItemizationEditor::FItemDefinitionAppModeInfo ModeArgs, TSharedRef<FItemDefinitionAppMode> ApplicationMode) = 0;

	/** Removes an ItemDefinition application mode. */
	UE_API virtual void RemoveItemDefinitionAppMode(FName ModeName) = 0;
};

#undef UE_API
