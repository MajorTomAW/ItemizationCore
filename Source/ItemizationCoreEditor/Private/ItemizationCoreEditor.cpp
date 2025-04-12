#include "ItemizationCoreEditor.h"

#include "Customization/ItemAssetTypeIdCustomization.h"
#include "Widgets/SItemAssetTypePicker.h"

#define LOCTEXT_NAMESPACE "FItemizationCoreEditorModule"

/** Module implementation of the ItemizationCoreEditor module */
class FItemizationCoreEditorModule final : public IItemizationCoreEditorModule
{
public:
	//~ Begin IModuleInterface Implementation
	virtual void StartupModule() override;
	virtual void PostLoadCallback() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Implementation

	virtual TSharedRef<SWidget> CreateItemAssetTypePicker(FOnItemAssetTypePicked& OnTypePicked) override;
};
IMPLEMENT_MODULE(FItemizationCoreEditorModule, ItemizationCoreEditor)

void FItemizationCoreEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEdModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyEdModule.RegisterCustomPropertyTypeLayout("ItemAssetTypeId",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FItemAssetTypeIdCustomization::MakeInstance));
	PropertyEdModule.NotifyCustomizationModuleChanged();
}

void FItemizationCoreEditorModule::PostLoadCallback()
{
}

void FItemizationCoreEditorModule::ShutdownModule()
{
}

TSharedRef<SWidget> FItemizationCoreEditorModule::CreateItemAssetTypePicker(FOnItemAssetTypePicked& OnTypePicked)
{
	return SNew(SItemAssetTypePicker)
		.OnItemAssetTypePickedDelegate(OnTypePicked);
}

#undef LOCTEXT_NAMESPACE
