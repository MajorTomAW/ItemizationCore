#include "ItemizationEditorModule.h"

#include "ContentBrowserMenuContexts.h"
#include "ItemDefinitionThumbnailRenderer.h"
#include "Commands/ItemDefinitionAppCommands.h"
#include "Items/ItemDefinitionBase.h"
#include "Styles/ItemizationEditorStyle.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Toolkits/ItemDefinitionApp.h"
#include "Toolkits/Modes/ItemDefinitionAppMode.h"
#include "Toolkits/Modes/Factories/ItemDefinitionAppModeFactory_DataList.h"
#include "Toolkits/Modes/Factories/ItemDefinitionAppModeFactory_Default.h"

#define LOCTEXT_NAMESPACE "FItemizationEditorModule"

namespace MenuExtension_ItemDefinition
{
	static void ExecuteCopyPrimaryAssetIdToClipboard(const FToolMenuContext& InContext)
	{
		const UContentBrowserAssetContextMenuContext* Context =
			UContentBrowserAssetContextMenuContext::FindContextWithAssets(InContext);

		// Example logic for executing a function on every selected asset of a specific type
		for (UItemDefinitionBase* Item : Context->LoadSelectedObjects<UItemDefinitionBase>())
		{
			Item->CopyPrimaryAssetIdToClipboard();
		}
	}

	static FDelayedAutoRegisterHelper DelayedAutoRegister(EDelayedRegisterRunPhase::EndOfEngineInit, []
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
		{
			FToolMenuOwnerScoped OwnerScoped(UE_MODULE_NAME);

			// Extend the menu of UItemDefinitionBase (this is a custom class I made)
			UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UItemDefinitionBase::StaticClass());

			FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
			Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
			{
				const TAttribute<FText> Label = LOCTEXT("ItemDefinitionCopyId", "Copy PrimaryAssetIds to Clipboard");
				const TAttribute<FText> ToolTip = LOCTEXT("ItemDefinitionCopyIdToolTip", "Copy the PrimaryAssetIds for the selected Item Definitions to the system clipboard. If multiple assets selected, each PrimaryAssetId is separated by a newline character.");

				FToolUIAction UIAction;
				UIAction.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&ExecuteCopyPrimaryAssetIdToClipboard);
				InSection.AddMenuEntry("ItemDefinition_ExecuteCopyPrimaryAssetIdToClipboard", Label, ToolTip, FSlateIcon(), UIAction);
			}));
		}));
	});
}

class FItemizationEditorModule final : public IItemizationEditorModule
{
	using ThisClass = FItemizationEditorModule;

public:
	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Interface

	//~ Begin IHasMenuExtensibility Interface
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override { return MenuExtensibilityManager; }
	//~ End IHasMenuExtensibility Interface

	//~ Begin IHasToolBarExtensibility Interface
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override { return ToolBarExtensibilityManager; }
	//~ End IHasToolBarExtensibility Interface

	virtual TSharedRef<IItemDefinitionApp> CreateItemDefinitionApp(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UItemDefinitionBase* ItemDefinition) override;

private:
	void OnPostEngineInit();

private:
	TArray<TSharedPtr<IItemDefinitionAppModeFactory>> DefaultModeFactories;
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;
};
IMPLEMENT_MODULE(FItemizationEditorModule, ItemizationEditor)


void FItemizationEditorModule::StartupModule()
{
	// Register the extensibility managers
	MenuExtensibilityManager = MakeShared<FExtensibilityManager>();
	ToolBarExtensibilityManager = MakeShared<FExtensibilityManager>();


	// Unregister existing thumbnail renderer if any.
	UThumbnailManager::Get().UnregisterCustomRenderer(UItemDefinitionBase::StaticClass());

	// Register our custom one
	UThumbnailManager::Get().RegisterCustomRenderer(UItemDefinitionBase::StaticClass(), UItemDefinitionThumbnailRenderer::StaticClass());

	// Initialize our slate style set
	FItemizationEditorStyle::Register();
	FItemDefinitionAppCommands::Register();

	// Assign to post-engine init delegate
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &ThisClass::OnPostEngineInit);
}

void FItemizationEditorModule::ShutdownModule()
{
	// Unassign from post-engine init delegate
	FCoreDelegates::OnPostEngineInit.RemoveAll(this);

	FItemizationEditorStyle::Unregister();
	FItemDefinitionAppCommands::Unregister();
}

TSharedRef<IItemDefinitionApp> FItemizationEditorModule::CreateItemDefinitionApp(
	const EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost,
	UItemDefinitionBase* ItemDefinition)
{
	TSharedRef<FItemDefinitionApp> App = MakeShared<FItemDefinitionApp>();
	App->InitApp(Mode, InitToolkitHost, ItemDefinition);
	return App;
}

void FItemizationEditorModule::OnPostEngineInit()
{
	// Register the default item definition editor mode factories
	DefaultModeFactories.Add(MakeShared<FItemDefinitionAppModeFactory_Default>());
	DefaultModeFactories.Add(MakeShared<FItemDefinitionAppModeFactory_DataList>());

	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	for (TSharedPtr Factory : DefaultModeFactories)
	{
		ModularFeatures.RegisterModularFeature(IItemDefinitionAppModeFactory::ModularFeatureName, Factory.Get());
	}

	// Register property type customizations
	/*@TODO: FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditor.RegisterCustomPropertyTypeLayout("ItemComponentDataInstance",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FItemComponentDataCustomization::MakeInstance));

	PropertyEditor.NotifyCustomizationModuleChanged();*/
}

#undef LOCTEXT_NAMESPACE
