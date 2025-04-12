using UnrealBuildTool;

public class ItemizationCoreEditor : ModuleRules
{
    public ItemizationCoreEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new []
        { 
            "Core",
        });

        PrivateDependencyModuleNames.AddRange(new []
        { 
            "CoreUObject", 
            "Engine", 
            "Slate", 
            "SlateCore",
            "ItemizationCoreRuntime",
            "AssetDefinition",
            "UnrealEd",
            "PropertyEditor",
            "InputCore",
        });
    }
}