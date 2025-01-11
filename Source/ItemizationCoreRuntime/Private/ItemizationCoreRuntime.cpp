#include "ItemizationCoreRuntime.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#endif

#include "ActorComponents/InventoryManager.h"
#include "GameFramework/HUD.h"
#include "GameplayDebugger/GameplayDebuggerCategory_Itemization.h"

#define LOCTEXT_NAMESPACE "FItemizationCoreRuntimeModule"

class FItemizationCoreRuntimeModule : public IItemizationCoreRuntimeModule
{
public:
	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Interface
};
IMPLEMENT_MODULE(FItemizationCoreRuntimeModule, ItemizationCoreRuntime)





void FItemizationCoreRuntimeModule::StartupModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& GameplayDebugger = IGameplayDebugger::Get();
	GameplayDebugger.RegisterCategory(
		"Itemization",
		IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_Itemization::MakeInstance),
		EGameplayDebuggerCategoryState::EnabledInGame,
		3);
	GameplayDebugger.NotifyCategoriesChanged();
#endif

	if (!IsRunningDedicatedServer())
	{
		AHUD::OnShowDebugInfo.AddStatic(&UInventoryManager::OnShowDebugInfo);
	}
}

void FItemizationCoreRuntimeModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebugger = IGameplayDebugger::Get();
		GameplayDebugger.UnregisterCategory("Itemization");
		GameplayDebugger.NotifyCategoriesChanged();
	}
#endif
}

#undef LOCTEXT_NAMESPACE