using System.IO;
using UnrealBuildTool;

public class ItemizationEditor : ModuleRules
{
    public ItemizationEditor(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange( new[]
        { 
	        "Core",
	        "AssetDefinition",
	        "ItemizationCore",
	        "PropertyEditor",
	        "EditorFramework",
        });

        var enginePath = Path.GetFullPath(target.RelativeEnginePath);
        PrivateIncludePaths.AddRange(new[]
        {
	        Path.Combine(enginePath, "Source/Editor/StructUtilsEditor/Private/"),
	        Path.Combine(enginePath, "Source/Editor/UnrealEd/Private/Toolkits/"),
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
        });
    }
}