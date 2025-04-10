#include "ItemizationCoreModule.h"

#include "ItemizationLogChannels.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"

static const FName NAME_ItemizationDebuggerCategory = FName("Itemization");
#endif

#define LOCTEXT_NAMESPACE "ItemizationCore"

/** Module implementation of the ItemizationCoreRuntime module */
class FItemizationCoreModule final : public IItemizationCoreModule
{
public:
	//~ Begin IModuleInterface Implementation
	virtual void StartupModule() override;
	virtual void PostLoadCallback() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Implementation

private:
};
IMPLEMENT_MODULE(FItemizationCoreModule, ItemizationCoreRuntime)


void FItemizationCoreModule::StartupModule()
{
	if (!IsRunningDedicatedServer())
	{
		
	}
}

void FItemizationCoreModule::PostLoadCallback()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebugger = IGameplayDebugger::Get();
		//GameplayDebugger.RegisterCategory(NAME_ItemizationDebuggerCategory,)
		GameplayDebugger.NotifyCategoriesChanged();
	}
#endif
}

void FItemizationCoreModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebugger = IGameplayDebugger::Get();
		GameplayDebugger.UnregisterCategory(NAME_ItemizationDebuggerCategory);
		GameplayDebugger.NotifyCategoriesChanged();
	}
#endif
}

#undef LOCTEXT_NAMESPACE