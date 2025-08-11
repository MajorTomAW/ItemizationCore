// Author: Tom Werner (MajorT), 2025

#include "ItemizationCoreModule.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "GameplayDebugger/GameplayDebuggerCategory_Itemization.h"
#endif

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ItemizationCoreSettings.h"
#endif

#define LOCTEXT_NAMESPACE "ItemizationCore"

class FItemizationCoreModule : public IItemizationCoreModule
{
public:
	//~ Begin IModuleInterface Implementation
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Implementation

#if WITH_EDITOR
private:
	ISettingsModule* SettingsModule = nullptr;
#endif
};
IMPLEMENT_MODULE(FItemizationCoreModule, ItemizationCore)

void FItemizationCoreModule::StartupModule()
{
#if WITH_EDITOR
	SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (ensure(SettingsModule))
	{
		SettingsModule->RegisterSettings(
			"Project",
			"Game",
			"Itemization Core",
			LOCTEXT("ItemizationCoreSettingsName", "Itemization Core"),
			LOCTEXT("ItemizationCoreSettingsDescription", "Configure the itemization core settings."),
			UItemizationCoreSettings::GetMutable());
	}
#endif

#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& GameplayDebugger = IGameplayDebugger::Get();
	GameplayDebugger.RegisterCategory(
		"Itemization",
		IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_Itemization::MakeInstance),
		EGameplayDebuggerCategoryState::EnabledInGame,
		3);
	GameplayDebugger.NotifyCategoriesChanged();
#endif
}

void FItemizationCoreModule::ShutdownModule()
{
#if WITH_EDITOR
	if (SettingsModule == nullptr)
	{
		SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		if (!ensure(SettingsModule))
		{
			return;
		}
	}

	SettingsModule->UnregisterSettings("Project", "Game", "Itemization Core");
#endif

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