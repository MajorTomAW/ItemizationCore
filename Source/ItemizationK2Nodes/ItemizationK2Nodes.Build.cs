using UnrealBuildTool;

public class ItemizationK2Nodes : ModuleRules
{
    public ItemizationK2Nodes(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "ItemizationCoreRuntime",
                "BlueprintGraph",
                "KismetCompiler",
                "UnrealEd",
            }
        );
    }
}