using UnrealBuildTool;

public class ItemizationCoreEditor : ModuleRules
{
    public ItemizationCoreEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new []
        {
            "Core",
            "UnrealEd",
            "EditorSubsystem",
            "DeveloperSettings",
            "AssetDefinition",
            "BlueprintGraph",
        });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "InputCore",
                "AssetDefinition",
                "ToolMenus",
                "UnrealEd",
                "PropertyEditor",
                "ContentBrowser",
                "EditorFramework",
                "AdvancedPreviewScene",
                "PropertyEditor",
                "WorkspaceMenuStructure",
                "ContentBrowserData",
                "ItemizationCoreRuntime",
                "Projects",
                "AssetManagerEditor",
                "DataValidation",
                "StructUtilsEditor",
            }
        );
    }
}