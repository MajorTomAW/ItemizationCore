#include "ItemizationCoreEditor.h"

#include "ContentBrowserMenuContexts.h"
#include "GameDelegates.h"
#include "ItemDefinition.h"
#include "ItemizationCoreEditorHelpers.h"
#include "Commands/ItemizationEditorCommands.h"
#include "Components/ItemComponentData_Equipment.h"
#include "Customization/ItemComponentDataCustomization.h"
#include "LevelEditor/ItemizationLevelEditorToolbar.h"
#include "Styles/ItemizationEditorStyle.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Thumbnails/ItemDefinitionThumbnailRenderer.h"
#include "Toolkits/ItemizationEditorApplication.h"
#include "Toolkits/ItemizationEditorAppMode.h"
#include "Toolkits/Modes/ItemizationAppMode_Components.h"
#include "Toolkits/Modes/ItemizationAppMode_Default.h"
#include "Toolkits/Modes/ItemizationAppMode_Equipment.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"

#define LOCTEXT_NAMESPACE "ItemizationCoreEditorModule"

using namespace UE::ItemizationCore::Editor;

//////////////////////////////////////////////////////////////////////////
/// FItemizationCoreEditorHelpers

void FItemizationEditorAssetConfig::SetRuleSet(const TSharedPtr<FItemizationEditorAssetRuleSet>& InRuleSet)
{
	RuleSet = InRuleSet;
}

bool FItemizationEditorAssetConfig::CanShowAppMode(const FName& ModeId) const
{
	if (!RuleSet->AllowedModeIds.IsEmpty())
	{
		return RuleSet->AllowedModeIds.Contains(ModeId);
	}
			
	return !RuleSet->DisallowedModeIds.Contains(ModeId);
}

bool FItemizationEditorAssetConfig::CanShowItemComponent(const FName& ComponentName) const
{
	if (!RuleSet->AllowedItemComponents.IsEmpty())
	{
		return RuleSet->AllowedItemComponents.Contains(ComponentName);
	}
			
	return !RuleSet->DisallowedItemComponents.Contains(ComponentName);
}

UStaticMesh* FItemizationEditorAssetConfig::FindPreviewMesh(const UItemDefinition* Item) const
{
	return nullptr;
}








//////////////////////////////////////////////////////////////////////////
/// FItemizationCoreEditorModule


class FItemizationCoreEditorModule final : public IItemizationCoreEditorModule
{
public:
	//~ Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface

	//~ Begin IItemizationCoreEditorModule
	virtual TSharedRef<FWorkflowCentricApplication> CreateItemizationApplication(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UObject* InObject) override;
	virtual void RegisterApplicationMode(UClass* AssetClass, FOnGetApplicationMode& OnGetApplicationMode, FName ModeId, FName CommandId = FName()) override;
	virtual void RegisterAssetConfig(const TSharedPtr<FItemizationEditorAssetConfig>& AssetConfig) override;
	virtual const TSharedPtr<FItemizationEditorAssetConfig> GetAssetConfig(const UClass* AssetClass) const override;
	virtual bool FindApplicationModesForAsset(UClass* AssetClass, TArray<FOnGetApplicationMode>& OutGetApplicationModes, bool bExactMatch = false) override;
	virtual TArray<FOnGetApplicationMode> GetAllApplicationModes() const override;
	virtual int32 GetNumApplicationModes() const override;
	virtual const TMap<FName, FName>& GetAllModeIds() const override { return AllModeIds; }
	//~ End IItemizationCoreEditorModule

private:
	/** Called right after the engine has been initialized */
	void OnPostEngineInit();

	/** Called right before the engine exits */
	void OnPreEngineExit();

	/** Called to register the default application modes. */
	void RegisterDefaultApplicationModes();

	/** List of delegates to call to get the application mode for a specific asset class */
	typedef TArray<FOnGetApplicationMode> FOnGetApplicationModeArray;
	TMap</*ClassName*/ FName, FOnGetApplicationModeArray> RegisteredApplicationModes;
	TMap</*ModeId*/ FName, /*CommandId*/ FName> AllModeIds;

	TArray<TSharedPtr<FItemizationEditorAssetConfig>> AssetConfigs;
};
IMPLEMENT_MODULE(FItemizationCoreEditorModule, ItemizationCoreEditor)

//********************************************************************************************************************
//
//		Module Implementation
//
//********************************************************************************************************************


void FItemizationCoreEditorModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FItemizationCoreEditorModule::OnPostEngineInit);
	FCoreDelegates::OnEnginePreExit.AddRaw(this, &FItemizationCoreEditorModule::OnPreEngineExit);
	
	FItemizationEditorStyle::Get();
}

void FItemizationCoreEditorModule::ShutdownModule()
{
	FCoreDelegates::OnPostEngineInit.RemoveAll(this);
	FCoreDelegates::OnEnginePreExit.RemoveAll(this);

	FItemizationEditorCommands::Unregister();
}

void FItemizationCoreEditorModule::OnPostEngineInit()
{
	RegisterDefaultApplicationModes();
	
	// Customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout
	(
		"ItemComponentDataProxy",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FItemComponentDataCustomization::MakeInstance)
	);

	// Thumbnail Renderer
	if (GIsEditor)
	{
		UThumbnailManager::Get().RegisterCustomRenderer(UItemDefinition::StaticClass(), UItemDefinitionThumbnailRenderer::StaticClass());
	}

	// Toolbar
	//FItemizationLevelEditorToolbar::RegisterItemizationLevelEditorToolBar();
	FItemizationEditorCommands::Register();
}

void FItemizationCoreEditorModule::OnPreEngineExit()
{
}

void FItemizationCoreEditorModule::RegisterDefaultApplicationModes()
{
	// Register default asset config
	{
		TSharedPtr<FItemizationEditorAssetConfig> DefaultConfig = MakeShared<FItemizationEditorAssetConfig>();
		DefaultConfig->AssetName = UItemDefinition::StaticClass()->GetFName();

		TSharedPtr<FItemizationEditorAssetRuleSet> RuleSet = MakeShared<FItemizationEditorAssetRuleSet>();
		RuleSet->AddAllowedModeIds({IDs::AppMode_Components(), IDs::AppMode_Equipment(), IDs::AppMode_Default()});
		DefaultConfig->SetRuleSet(RuleSet);

		RegisterAssetConfig(DefaultConfig);
	}
	
	{ // Default Mode
		FOnGetApplicationMode OnGetApplicationMode
		(
			FOnGetApplicationMode::CreateLambda([](TSharedPtr<FApplicationMode>& OutMode, const TSharedPtr<FWorkflowCentricApplication>& App)->FItemizationAppModeArgs
			{
				FItemizationAppModeArgs Args
				(
					IDs::AppMode_Default(),
					LOCTEXT("DefaultModeLabel", "Defaults"),
					FText::Format(
						LOCTEXT("DefaultModeTooltip", "The default settings for the item. ({0})"),
						FItemizationEditorCommands::Get().AppMode_Default->GetInputText()),
					FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Details"),
					INT_MIN
				);

				TSharedPtr<FItemizationEditorApplication> MutableApp = StaticCastSharedPtr<FItemizationEditorApplication>(App);
				OutMode = MakeShared<FItemizationAppMode_Default>(MutableApp, Args);

				return Args;
			})
		);

		RegisterApplicationMode(UItemDefinition::StaticClass(), OnGetApplicationMode, IDs::AppMode_Default());
	}

	{ // Components Mode
		FOnGetApplicationMode OnGetApplicationMode
		(
			FOnGetApplicationMode::CreateLambda([](TSharedPtr<FApplicationMode>& OutMode, const TSharedPtr<FWorkflowCentricApplication>& App)->FItemizationAppModeArgs
			{
				FItemizationAppModeArgs Args
				(
					IDs::AppMode_Components(),
					LOCTEXT("ComponentsModeLabel", "Components"),
					FText::Format(
						LOCTEXT("ComponentsModeTooltip", "Manage the components of the item. ({0})"),
						FItemizationEditorCommands::Get().AppMode_Components->GetInputText()),
					FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Components"),
					-1
				);

				TSharedPtr<FItemizationEditorApplication> MutableApp = StaticCastSharedPtr<FItemizationEditorApplication>(App);
				OutMode = MakeShared<FItemizationAppMode_Components>(MutableApp, Args);

				return Args;
			})
		);

		RegisterApplicationMode(UItemDefinition::StaticClass(), OnGetApplicationMode, IDs::AppMode_Components());
	}

	{ // Equipment Mode
		FOnGetApplicationMode OnGetApplicationMode
		(
			FOnGetApplicationMode::CreateLambda([](TSharedPtr<FApplicationMode>& OutMode, const TSharedPtr<FWorkflowCentricApplication>& App)->FItemizationAppModeArgs
			{
				FItemizationAppModeArgs Args
				(
					IDs::AppMode_Equipment(),
					LOCTEXT("EquipmentModeLabel", "Equipment"),
					FText::Format(
						LOCTEXT("EquipmentModeTooltip", "Manage the equipment of the item. ({0})"),
						FItemizationEditorCommands::Get().AppMode_Equipment->GetInputText()),
					FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.Equipment"),
					0
				);

				TSharedPtr<FItemizationEditorApplication> MutableApp = StaticCastSharedPtr<FItemizationEditorApplication>(App);
				OutMode = MakeShared<FItemizationAppMode_Equipment>(MutableApp, Args);

				return Args;
			})
		);

		RegisterApplicationMode(UItemDefinition::StaticClass(), OnGetApplicationMode, IDs::AppMode_Equipment());
	}
}

void FItemizationCoreEditorModule::RegisterAssetConfig(
	const TSharedPtr<FItemizationEditorAssetConfig>& AssetConfig)
{
	if (AssetConfig == nullptr || AssetConfig->AssetName == NAME_None)
	{
		return;
	}

	AssetConfigs.Add(AssetConfig);
}

const TSharedPtr<FItemizationEditorAssetConfig> FItemizationCoreEditorModule::GetAssetConfig(const UClass* AssetClass) const
{
	if (AssetClass == nullptr)
	{
		return nullptr;
	}

	if (AssetConfigs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No asset configs registered"));
		return nullptr;
	}
		
	// Recursively search for the asset config that matches exactly the asset class or its parent
	const UClass* SuperClass = AssetClass;
	while (SuperClass && ( SuperClass->IsChildOf(UItemDefinition::StaticClass()) || SuperClass == UItemDefinition::StaticClass()) )
	{
		for (const TSharedPtr Config : AssetConfigs)
		{
			if (Config->AssetName == SuperClass->GetFName())
			{
				return Config;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("No asset config found for class %s"), *SuperClass->GetName());
		SuperClass = SuperClass->GetSuperClass();
	}

	return nullptr;
}

TSharedRef<FWorkflowCentricApplication> FItemizationCoreEditorModule::CreateItemizationApplication(
	const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UObject* InObject)
{
	TSharedRef<FItemizationEditorApplication> App = MakeShared<FItemizationEditorApplication>();
	App->InitItemizationEditorApp(Mode, InitToolkitHost, Cast<UItemDefinition>(InObject));
	return App;
}

void FItemizationCoreEditorModule::RegisterApplicationMode(
	UClass* AssetClass, FOnGetApplicationMode& OnGetApplicationMode, FName ModeId, FName CommandId)
{
	if (AssetClass == nullptr)
	{
		return;
	}

	if (!OnGetApplicationMode.IsBound())
	{
		return;
	}

	const FName AssetClassName = AssetClass->GetFName();
	FOnGetApplicationModeArray& AppModes = RegisteredApplicationModes.FindOrAdd(AssetClassName);
	AppModes.Add(OnGetApplicationMode);
	
	FName& NewCommandId = AllModeIds.Add(ModeId);
	if (CommandId.IsNone())
	{
		NewCommandId = ModeId;
	}
	else
	{
		NewCommandId = CommandId;
	}
}

bool FItemizationCoreEditorModule::FindApplicationModesForAsset(
	UClass* AssetClass, TArray<FOnGetApplicationMode>& OutGetApplicationModes, bool bExactMatch)
{
	OutGetApplicationModes.Reset();
	
	if (AssetClass == nullptr)
	{
		return false;
	}

	bool bFoundAny = false;

	const FName AssetClassName = AssetClass->GetFName();
	if (const FOnGetApplicationModeArray* AppModes = RegisteredApplicationModes.Find(AssetClassName))
	{
		OutGetApplicationModes.Append(*AppModes);
		bFoundAny = AppModes->Num() > 0;
	}

	// Iterate through all registered application modes and check if the asset class is a parent of the registered class
	if (!bExactMatch)
	{
		UClass* SuperClass = AssetClass->GetSuperClass();
		while (SuperClass)
		{
			const FName SuperClassName = SuperClass->GetFName();
			if (const FOnGetApplicationModeArray* AppModes = RegisteredApplicationModes.Find(SuperClassName))
			{
				OutGetApplicationModes.Append(*AppModes);
				bFoundAny = AppModes->Num() > 0;
			}

			SuperClass = SuperClass->GetSuperClass();
		}
	}

	return bFoundAny;
}

TArray<IItemizationCoreEditorModule::FOnGetApplicationMode> FItemizationCoreEditorModule::GetAllApplicationModes() const
{
	TArray<FOnGetApplicationMode> FilteredModes;

	for (const auto& KVP : RegisteredApplicationModes)
	{
		KVP.Value.FilterByPredicate([&FilteredModes](const FOnGetApplicationMode& Mode)
		{
			FilteredModes.Add(Mode);
			return false;
		});
	}

	return FilteredModes;
}

int32 FItemizationCoreEditorModule::GetNumApplicationModes() const
{
	int32 NumModes = 0;
	for (const auto& KVP : RegisteredApplicationModes)
	{
		if (NumModes < KVP.Value.Num())
		{
			NumModes = KVP.Value.Num();
		}
	}
	return NumModes;
}

#undef LOCTEXT_NAMESPACE
