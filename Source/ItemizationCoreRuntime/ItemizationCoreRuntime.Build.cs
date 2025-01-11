using UnrealBuildTool;

public class ItemizationCoreRuntime : ModuleRules
{
    public ItemizationCoreRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        NumIncludedBytesPerUnityCPPOverride = 491520; // best unity size found from using UBT ProfileUnitySizes mode
        
        // Turn this off if you aren't planning on using the GameplayMessageRouter
        PublicDefinitions.Add("WITH_GAMEPLAY_MESSAGE_ROUTER=0");
        
        // Turn this off if you aren't planning on using the EquipmentSystem
        PublicDefinitions.Add("WITH_EQUIPMENT_SYSTEM=0");

        PublicDependencyModuleNames.AddRange(new []
        {
            "Core",
            "StructUtils",
            "NetCore",
            "Iris",
            "IrisCore",
            "GameplayTags",
            "GameplayAbilities",
            "DeveloperSettings",
        });

        PrivateDependencyModuleNames.AddRange(new []
        {
            "CoreUObject",
            "Engine",
            "EngineSettings"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("Slate");
            PrivateDependencyModuleNames.Add("SlateCore");
        }

        if (PublicDefinitions.Contains("WITH_GAMEPLAY_MESSAGE_ROUTER=1"))
        {
            PrivateDependencyModuleNames.Add("GameplayMessageRuntime");
        }
        
        SetupGameplayDebuggerSupport(Target);
        
        SetupIrisSupport(Target);
    }
}