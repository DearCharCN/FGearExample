// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;

public class FGearPlugin : ModuleRules
{
	public FGearPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				"FGearPlugin/Private"
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"ProceduralMeshComponent",
				"EnhancedInput",
				"PhysicsCore", //for 4.26+
				"NetCore",
				"AIModule",
				"Niagara"
				// ... add other public dependencies that you statically link with here ...
			}
			);

        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Json",
                "JsonUtilities"
				// ... add private dependencies that you statically link with here ...	
			}
			);

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
				new string[] 
				{ 
					"DetailCustomizations",
					"PropertyEditor",
					"EditorStyle",
					"DeveloperSettings" //for 4.26+
				}
				);
            PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		SetupModulePhysicsSupport(Target);
		PrivateDefinitions.Add("CHAOS_INCLUDE_LEVEL_1=1");
	}
}
