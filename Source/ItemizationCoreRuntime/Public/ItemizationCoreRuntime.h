#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IItemizationCoreRuntimeModule : public IModuleInterface
{
public:
    /**
     * Singleton-like access to this module's interface. This is just for convenience!
     * Beware of calling this during the shutdown phase, though. Your module might have been unloaded already.
     *
     * @returns Singleton instance, loading the module on demand if needed
     */
	static IItemizationCoreRuntimeModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IItemizationCoreRuntimeModule>("ItemizationCoreRuntime");
	}
};
