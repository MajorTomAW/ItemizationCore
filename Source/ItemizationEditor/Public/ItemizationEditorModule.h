#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UItemDefinitionBase;
class IItemDefinitionApp;
constexpr char ModuleName[] = "ItemizationEditor";

#define UE_API ITEMIZATIONEDITOR_API

class IItemizationEditorModule
	: public IModuleInterface
	, public IHasMenuExtensibility
	, public IHasToolBarExtensibility
{
public:
	/** Gets this module, will attempt to load and should always exist. */
	UE_API static IItemizationEditorModule& GetModule()
	{
		return FModuleManager::LoadModuleChecked<IItemizationEditorModule>(ModuleName);
	}

	/** Gets this module, will not attempt to load and may not exist. */
	UE_API static IItemizationEditorModule* GetModulePtr()
	{
		return FModuleManager::GetModulePtr<IItemizationEditorModule>(ModuleName);
	}

	/**
	 * Checks to see if this module is loaded and ready.
	 *
	 * @returns True if the module is loaded and ready to use
	 */
	UE_API static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(ModuleName);
	}

public:
	/** Creates an instance of the ItemDefinition editor. */
	virtual TSharedRef<IItemDefinitionApp> CreateItemDefinitionApp(
		const EToolkitMode::Type Mode,
		const TSharedPtr<IToolkitHost>& InitToolkitHost,
		UItemDefinitionBase* ItemDefinition) = 0;
};

#undef UE_API
