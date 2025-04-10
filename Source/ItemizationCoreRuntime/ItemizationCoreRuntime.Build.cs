using UnrealBuildTool;

public class ItemizationCoreRuntime : ModuleRules
{
    public ItemizationCoreRuntime(ReadOnlyTargetRules target) : base(target)
    {
        // Turn this off if you aren't planning on using the GameplayMessageRouter
        const bool bUseGameplayMessaging = true;
        
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        NumIncludedBytesPerUnityCPPOverride = 491520; // best unity size found from using UBT ProfileUnitySizes mode

        PublicDependencyModuleNames.AddRange(new []
        {
            "Core",
            "CoreUObject",
            "NetCore",
            "Engine",
            "GameplayTags",
            "GameplayTagStacks",
            "GameplayAbilities",
            "ModularGameplay",
            "EnhancedInput"
        });
        
        PrivateDependencyModuleNames.AddRange(new []
        {
            "EngineSettings",
        });
        
        // Expose our module to other relevant modules
        SetupGameplayDebuggerSupport(target);
        SetupIrisSupport(target);
        
        // Include internal dependencies
        IncludeEditorOnlyDependencies(target, this);

        if (bUseGameplayMessaging)
        {
            IncludeGameplayMessagingDependencies(this);
        }
    }
    
    internal static void IncludeEditorOnlyDependencies(ReadOnlyTargetRules target, ModuleRules module)
    {
        if (!target.bBuildEditor)
        { 
            return;
        }
        
        module.PrivateDependencyModuleNames.AddRange(new []
        {
            "EditorFramework",
            "UnrealEd",
            "Slate",
        });
    }
    
    internal static void IncludeGameplayMessagingDependencies(ModuleRules module)
    {
        module.PublicDefinitions.Add("WITH_GAMEPLAY_MESSAGING=1");
        module.PublicDependencyModuleNames.Add("GameplayMessageRuntime");
    }
}