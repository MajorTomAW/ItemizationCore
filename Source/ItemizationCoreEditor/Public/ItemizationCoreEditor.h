#pragma once

#include "Modules/ModuleManager.h"

DECLARE_DELEGATE_OneParam(FOnItemAssetTypePicked, const FName& /*AssetTypeId*/)

/** The public interface of the ItemizationCoreEditor module. */
class IItemizationCoreEditorModule : public IModuleInterface
{
public:
    /**
     * Singleton-like access to this module's interface. This is just for convenience!
     * Be careful using this during the shutdown phase, though.
     * Your module might have been unloaded already.
     */
	static IItemizationCoreEditorModule& Get()
	{
		static IItemizationCoreEditorModule& Singleton = FModuleManager::LoadModuleChecked<IItemizationCoreEditorModule>("ItemizationCoreEditor");
		return Singleton;
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @returns True if the module is loaded and ready to use
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ItemizationCoreEditor");
	}

	/** Creates a new item asset type ID picker widget. */
	virtual TSharedRef<SWidget> CreateItemAssetTypePicker(FOnItemAssetTypePicked& OnTypePicked) = 0;
};
