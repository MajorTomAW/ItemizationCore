#include "ItemizationEditorModule.h"

#include "Customization/ItemComponentDataCustomization.h"
#include "Items/ItemDefinitionBase.h"
#include "Styles/ItemizationEditorStyle.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Thumbnails/ItemDefinitionThumbnailRenderer.h"
#include "Toolkits/ItemDefinitionApplication.h"
#include "Toolkits/ItemDefinitionApplicationMode.h"
#include "Toolkits/ModeFactories/ItemDefinitionAppModeFactory_DataList.h"
#include "Toolkits/ModeFactories/ItemDefinitionAppModeFactory_Default.h"

#define LOCTEXT_NAMESPACE "ItemizationEditor"

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

	//~ Begin IItemizationEditorModule Interface
	virtual TSharedRef<IItemDefinitionApplication> CreateItemDefinitionEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, class UItemDefinitionBase* ItemDefinition) override;

private:
	void OnPostEngineInit();

private:
	TArray<TSharedPtr<IItemDefinitionApplicationModeFactory>> DefaultModeFactories;
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
	FItemizationEditorStyle::Get();

	// Assign to post-engine init delegate
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &ThisClass::OnPostEngineInit);
}

void FItemizationEditorModule::ShutdownModule()
{
	// Unassign from post-engine init delegate
	FCoreDelegates::OnPostEngineInit.RemoveAll(this);
}

TSharedRef<IItemDefinitionApplication> FItemizationEditorModule::CreateItemDefinitionEditor(
	const EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost,
	UItemDefinitionBase* ItemDefinition)
{
	TSharedRef<FItemDefinitionApplication> NewEditor = MakeShared<FItemDefinitionApplication>();
	NewEditor->InitEditor(Mode, InitToolkitHost, ItemDefinition);
	return NewEditor;
}

void FItemizationEditorModule::OnPostEngineInit()
{
	// Register the default item definition editor mode factories
	DefaultModeFactories.Add(MakeShared<FItemDefinitionAppModeFactory_Default>());
	DefaultModeFactories.Add(MakeShared<FItemDefinitionAppModeFactory_DataList>());
	
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	for (TSharedPtr Factory : DefaultModeFactories)
	{
		ModularFeatures.RegisterModularFeature(IItemDefinitionApplicationModeFactory::ModularFeatureName, Factory.Get());
	}
	
	// Register property type customizations
	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditor.RegisterCustomPropertyTypeLayout("ItemComponentDataInstance",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FItemComponentDataCustomization::MakeInstance));

	PropertyEditor.NotifyCustomizationModuleChanged();
}

#undef LOCTEXT_NAMESPACE