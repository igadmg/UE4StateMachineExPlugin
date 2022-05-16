using System.IO;
using UnrealBuildTool;



public class StateMachineEx : ModuleRules
{
	public StateMachineEx(ReadOnlyTargetRules Target)
		: base(Target)
	{
		bEnforceIWYU = true;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		PrivatePCHHeaderFile = "Private/StateMachineExPrivatePCH.h";


		PublicIncludePaths.AddRange(new string[] {
				Path.Combine(ModuleDirectory, "Public"),
			});


		PrivateIncludePaths.AddRange(new string[] {
				Path.Combine(ModuleDirectory, "Private"),
			});


		PublicDependencyModuleNames.AddRange(new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"UMG",
			});

		PublicDependencyModuleNames.AddRange(new string[] {
				"CoreEx",
				"ComponentEx",
				"UnrealEngineEx",
			});


		PrivateDependencyModuleNames.AddRange(new string[] {
			});

		PrivateDependencyModuleNames.AddRange(new string[] {
			});


		DynamicallyLoadedModuleNames.AddRange(new string[]{});
	}
}
