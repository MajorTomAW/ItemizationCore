using UnrealBuildTool;

public class ItemizationCoreRuntime : ModuleRules
{
    public ItemizationCoreRuntime(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        NumIncludedBytesPerUnityCPPOverride = 491520; // best unity size found from using UBT ProfileUnitySizes mode

        PublicDependencyModuleNames.AddRange(
        [
            "Core",
            "CoreUObject",
            "NetCore",
            "Engine",
            "GameplayTags",
            "GameplayAbilities",
            "EnhancedInput",
            "ModularGameplay",
            "GameFeatures",
        ]);

        PrivateDependencyModuleNames.AddRange(
        [
            "EngineSettings",
        ]);
        
        // Include internal dependencies
        IncludeEditorOnlyDependencies(target, this);
        
        // Expose our module to other relevant modules
        SetupGameplayDebuggerSupport(target);
        SetupIrisSupport(target);
    }

    internal static void IncludeEditorOnlyDependencies(ReadOnlyTargetRules target, ModuleRules module)
    {
        if (!target.bBuildEditor)
        {
            return;
        }
        
        module.PrivateDependencyModuleNames.AddRange([
            "AssetRegistry",
            "EditorFramework",
            "UnrealEd",
            "Slate",
        ]);
    }
}