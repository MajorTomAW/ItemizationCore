#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FWorkflowCentricApplication;
class FApplicationMode;

namespace UE::ItemizationCore::Editor::IDs
{
    ITEMIZATIONCOREEDITOR_API inline FName ItemizationEditorApp() { return "ItemizationEditorApp"; }

	ITEMIZATIONCOREEDITOR_API inline FName AppMode_Default() { return "AppMode_Default"; }
	ITEMIZATIONCOREEDITOR_API inline FName AppMode_Components() { return "AppMode_Components"; }
	ITEMIZATIONCOREEDITOR_API inline FName AppMode_Equipment() { return "AppMode_Equipment"; }

	ITEMIZATIONCOREEDITOR_API inline FName TabID_Details() { return "Details"; }
	ITEMIZATIONCOREEDITOR_API inline FName TabID_Viewport() { return "Viewport"; }
	ITEMIZATIONCOREEDITOR_API inline FName TabID_Placement() { return "Placement"; }
	ITEMIZATIONCOREEDITOR_API inline FName TabID_Development() { return "Development"; }
	ITEMIZATIONCOREEDITOR_API inline FName TabID_DisplayInfo() { return "DisplayInfo"; }
	ITEMIZATIONCOREEDITOR_API inline FName TabID_Components() { return "Components"; }
	ITEMIZATIONCOREEDITOR_API inline FName TabID_Equipment() { return "Equipment"; }
}

/**
 * The public interface to this module.
 * In most cases, this interface is only public to sibling modules within this plugin
 */
class IItemizationCoreEditorModule : public IModuleInterface
{
public:
	/** Args to define an app mode in the itemization editor. */
	ITEMIZATIONCOREEDITOR_API struct FItemizationAppModeArgs
	{
	public:
		FItemizationAppModeArgs()
			: Priority(INDEX_NONE)
			, ExtensionHookId("Modes")
		{
		}

		FItemizationAppModeArgs(const FName InModeId, const FText& InModeLabel, const FText& InModeTooltip = FText(), int32 InPriority = INDEX_NONE)
			: ModeId(InModeId)
			, Priority(InPriority)
			, ExtensionHookId("Modes")
			, ModeLabel(InModeLabel)
			, ModeTooltip(InModeTooltip)
		{
		}

		FItemizationAppModeArgs(const FName InModeId, const FText& InModeLabel, const FText& InModeTooltip = FText(), const FSlateIcon& InModeIcon = FSlateIcon(), int32 InPriority = INDEX_NONE)
			: ModeId(InModeId)
			, Priority(InPriority)
			, ExtensionHookId("Modes")
			, ModeLabel(InModeLabel)
			, ModeTooltip(InModeTooltip)
			, ModeIcon(InModeIcon)
		{
		}

		FItemizationAppModeArgs(const FName InModeId, const FName InExtensionHookId, const FText& InModeLabel, const FText& InModeTooltip = FText(), const FSlateIcon& InModeIcon = FSlateIcon(), int32 InPriority = INDEX_NONE)
			: ModeId(InModeId)
			, Priority(InPriority)
			, ExtensionHookId(InExtensionHookId)
			, ModeLabel(InModeLabel)
			, ModeTooltip(InModeTooltip)
			, ModeIcon(InModeIcon)
		{
		}
		
		/** Unique Id for the mode */
		FName ModeId;

		/** Priority of the mode */
		int32 Priority;

		/** The hook id to use for the mode switcher */
		FName ExtensionHookId;

		/** Label to display in the UI for the mode */
		FText ModeLabel;

		/** Tooltip to display in the UI for the mode */
		FText ModeTooltip;

		/** Icon to display in the UI for the mode */
		FSlateIcon ModeIcon;

		bool operator==(const FItemizationAppModeArgs& B) const
		{
			return ModeId == B.ModeId;
		}
	};
	DECLARE_DELEGATE_RetVal_TwoParams(FItemizationAppModeArgs /*OutArgs*/, FOnGetApplicationMode, TSharedPtr<FApplicationMode>& /*Mode*/, const TSharedPtr<FWorkflowCentricApplication>& /*OwningApp*/);

public:
	/**
	 * Singleton-like access to this module's interface.
	 * This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.
	 * Your module might have been unloaded already.
	 *
	 * @returns Singleton instance, loading the module on demand if needed
	 */
	ITEMIZATIONCOREEDITOR_API static IItemizationCoreEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IItemizationCoreEditorModule>("ItemizationCoreEditor");
	}

	/**
	 * Checks to see if this module is loaded and ready.
	 *
	 * @returns True if the module is loaded and ready to use
	 */
	ITEMIZATIONCOREEDITOR_API static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ItemizationCoreEditor");
	}

	/**
	 * Registers a new application mode for the itemization editor.
	 * Mapped to a specific class to allow different modes to be created.
	 */
	ITEMIZATIONCOREEDITOR_API virtual void RegisterApplicationMode(UClass* AssetClass, FOnGetApplicationMode& OnGetApplicationMode, FName ModeId, FName CommandId = FName()) = 0;

	/**
	 * Finds all application mode delegates mapped to the given asset class.
	 *
	 * @param AssetClass The class to find application modes for.
	 * @param OutGetApplicationModes List of application mode delegates found.
	 * @param bExactMatch If true, only exact matches will be returned. Meaning that subclasses of the given class will not be included.
	 * @returns True if any application modes were found.
	 */
	ITEMIZATIONCOREEDITOR_API virtual bool FindApplicationModesForAsset(UClass* AssetClass, TArray<FOnGetApplicationMode>& OutGetApplicationModes, bool bExactMatch = false) = 0;

	/**
	 * Returns all application mode delegates.
	 * Filtering out dupes and sorting by priority.
	 */
	ITEMIZATIONCOREEDITOR_API virtual TArray<FOnGetApplicationMode> GetAllApplicationModes() const = 0;

	/**
	 * Returns the number of application modes registered.
	 */
	ITEMIZATIONCOREEDITOR_API virtual int32 GetNumApplicationModes() const = 0;

	/**
	 * Returns the list of all registered app mode ids.
	 */
	ITEMIZATIONCOREEDITOR_API virtual const TMap<FName, FName>& GetAllModeIds() const = 0;

	/**
	 * Creates a new itemization editor application to edit the specified object.
	 * @returns The new application instance.
	 */
	ITEMIZATIONCOREEDITOR_API virtual TSharedRef<FWorkflowCentricApplication> CreateItemizationApplication(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UObject* InObject) = 0;
};
