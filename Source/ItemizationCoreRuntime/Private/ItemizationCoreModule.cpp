#include "ItemizationCoreModule.h"

#include "ISettingsModule.h"
#include "ItemizationCoreSettings.h"

#define LOCTEXT_NAMESPACE "FItemizationCoreRuntimeModule"

/** The implementation of the ItemizationCore module. */
class FItemizationCoreModule final : public IItemizationCoreModule
{
public:
	//~ Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface

private:
	ISettingsModule* SettingsModule = nullptr;
};
IMPLEMENT_MODULE(FItemizationCoreModule, ItemizationCoreRuntime)

void FItemizationCoreModule::StartupModule()
{
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
}

void FItemizationCoreModule::ShutdownModule()
{
	if (SettingsModule == nullptr)
	{
		SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		if (!ensure(SettingsModule))
		{
			return;
		}
	}

	SettingsModule->UnregisterSettings("Project", "Game", "Itemization Core");
}

#undef LOCTEXT_NAMESPACE