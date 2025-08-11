using UnrealBuildTool;

public class ItemizationK2Nodes : ModuleRules
{
    public ItemizationK2Nodes(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange( new[]
        { 
	        "Core",
        });

        PrivateDependencyModuleNames.AddRange( new [] 
	    { 
		    "CoreUObject", 
		    "Engine", 
		    "Slate", 
		    "SlateCore", 
		    "GraphEditor",
		    "StructUtilsEditor", 
		    "BlueprintGraph",
		    "UnrealEd",
		    "ItemizationCore"
	    });
    }
}