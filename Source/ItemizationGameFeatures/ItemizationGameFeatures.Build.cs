using UnrealBuildTool;

public class ItemizationGameFeatures : ModuleRules
{
    public ItemizationGameFeatures(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange( new[]
        { 
	        "Core",
	        "GameFeatures",
	        "ModularGameplay",
	        "ItemizationCore",
        });

        PrivateDependencyModuleNames.AddRange( new[]
        { 
	        "CoreUObject", 
	        "Engine", 
        });
    }
}