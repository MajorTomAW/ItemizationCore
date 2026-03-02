using UnrealBuildTool;

public class ItemizationCoreUI : ModuleRules
{
    public ItemizationCoreUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new []
        {
	        "Core",
	        "ModelViewViewModel",
	        "GameplayTags",
	        "ItemizationCore",
	        "UMG",
        });

        PrivateDependencyModuleNames.AddRange( new[]
        {
	        "CoreUObject",
	        "Engine",
        });
    }
}
