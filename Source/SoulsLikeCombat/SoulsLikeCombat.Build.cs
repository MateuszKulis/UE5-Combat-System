// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SoulsLikeCombat : ModuleRules
{
	public SoulsLikeCombat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "HeadMountedDisplay", "AIModule", "GameplayCameras", "Niagara" });
	}
}
