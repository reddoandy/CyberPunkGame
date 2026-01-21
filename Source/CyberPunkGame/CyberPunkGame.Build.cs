// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CyberPunkGame : ModuleRules
{
	public CyberPunkGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
			"Http",
            "Json",
			"JsonUtilities"
        });
	}


}

