using UnrealBuildTool;

public class ItemizationEditor : ModuleRules
{
    public ItemizationEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange( new[]
        {
	        "Core",
	        "AssetDefinition",
	        "ItemizationCore",
	        "PropertyEditor",
	        "EditorFramework",
	        "EditorSubsystem",
        });

        PrivateDependencyModuleNames.AddRange( new []
        {
	        "CoreUObject",
	        "Engine",
	        "Slate",
	        "SlateCore",
	        "Projects",
	        "UnrealEd",
	        "StructUtilsEditor",
	        "ToolWidgets",
	        "InputCore",
	        "ToolMenus",
	        "ToolWidgets",
	        "InteractiveToolsFramework",
	        "DataValidation",
	        "AssetManagerEditor",
	        "AdvancedPreviewScene",
	        "DeveloperSettings",
			"ContentBrowser",
        });
    }
}
