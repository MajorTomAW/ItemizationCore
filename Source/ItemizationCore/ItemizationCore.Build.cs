// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ItemizationCore : ModuleRules
{
	public ItemizationCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;


		PublicDependencyModuleNames.AddRange(new []
		{
			"Core",
			"NetCore",
			"Iris",
			"IrisCore",
			"ApplicationCore",
			"GameplayTags",
			"GameplayAbilities",
			"DeveloperSettings",
			"ModularGameplay",
		});
		PrivateDependencyModuleNames.AddRange(new []
		{
			"CoreUObject",
			"Engine",
			"EngineSettings",
			"GameFeatures",
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("Slate");
			PrivateDependencyModuleNames.Add("SlateCore");
		}

		SetupGameplayDebuggerSupport(Target);
		SetupIrisSupport(Target);
	}
}
