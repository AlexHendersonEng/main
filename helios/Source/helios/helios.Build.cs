// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class helios : ModuleRules
{
	public helios(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Lets code anywhere in the module include headers via folder-qualified paths, e.g. "Mesh/HeliosRuntimeMeshLoader.h".
		PublicIncludePaths.Add(ModuleDirectory);
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "ProceduralMeshComponent" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Json", "JsonUtilities", "LevelSequence", "MovieScene", "MovieSceneTracks" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
