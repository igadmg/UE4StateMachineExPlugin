using System.IO;
using UnrealBuildTool;



public class StateMachineDeveloperEx : ModuleRules
{
	public StateMachineDeveloperEx(ReadOnlyTargetRules Target)
		: base(Target)
	{
		bEnforceIWYU = true;
		DefaultBuildSettings = BuildSettingsVersion.V2;


		PublicIncludePaths.AddRange(new string[] {
				Path.Combine(ModuleDirectory, "Public"),
			});


		PrivateIncludePaths.AddRange(new string[] {
				Path.Combine(ModuleDirectory, "Private"),
			});


		PublicDependencyModuleNames.AddRange(new string[] {
				"BlueprintGraph",
				"Core",
				"CoreUObject",
				"Engine",
				"UnrealEd",
			});

		PublicDependencyModuleNames.AddRange(new string[] {
			});


		PrivateDependencyModuleNames.AddRange(new string[] {
				"KismetCompiler",
			});

		PrivateDependencyModuleNames.AddRange(new string[] {
				"StateMachineEx",
				"UnrealDeveloperEx",
			});


		DynamicallyLoadedModuleNames.AddRange(new string[]{});
	}
}
