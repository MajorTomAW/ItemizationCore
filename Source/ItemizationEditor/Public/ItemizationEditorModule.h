#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"

#define MY_API ITEMIZATIONEDITOR_API

constexpr char ModuleName[] = "ItemizationEditor";

class IItemDefinitionApplication;
class UItemDefinitionBase;

class IItemizationEditorModule
	: public IModuleInterface
	, public IHasMenuExtensibility
	, public IHasToolBarExtensibility
{
public:
	/** Gets this module, will attempt to load and should always exist. */
	MY_API static IItemizationEditorModule& GetModule()
	{
		return FModuleManager::LoadModuleChecked<IItemizationEditorModule>(ModuleName);
	}

	/** Gets this module, will not attempt to load and may not exist. */
	MY_API static IItemizationEditorModule* GetModulePtr()
	{
		return FModuleManager::GetModulePtr<IItemizationEditorModule>(ModuleName);
	}

	/**
	 * Checks to see if this module is loaded and ready.
	 *
	 * @returns True if the module is loaded and ready to use
	 */
	MY_API static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(ModuleName);
	}

public:
	/** Creates an instance of the ItemDefinition editor. */
	virtual TSharedRef<IItemDefinitionApplication> CreateItemDefinitionEditor(
		const EToolkitMode::Type Mode,
		const TSharedPtr<IToolkitHost>& InitToolkitHost,
		UItemDefinitionBase* ItemDefinition) = 0;
};

#undef MY_API