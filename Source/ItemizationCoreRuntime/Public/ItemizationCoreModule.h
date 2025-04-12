#pragma once

#include "Modules/ModuleManager.h"

/** The public interface to this module. */
class IItemizationCoreModule : public IModuleInterface
{
public:
    /**
     * Singleton-like access to this module's interface.  This is just for convenience!
     * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
     *
     * @returns Singleton-instance, loading the module on demand if needed
    */
    ITEMIZATIONCORERUNTIME_API static IItemizationCoreModule& Get()
    {
        static IItemizationCoreModule& Singleton = FModuleManager::LoadModuleChecked<IItemizationCoreModule>("ItemizationCoreRuntime");
        return Singleton;
    }

    /**
     * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
     *
     * @returns True if the module is loaded and ready to use
     */
    ITEMIZATIONCORERUNTIME_API static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("ItemizationCoreRuntime");
    }
};
