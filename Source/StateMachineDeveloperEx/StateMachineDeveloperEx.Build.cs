using System.IO;
using UnrealBuildTool;



public class StateMachineDeveloperEx : ModuleRules
{
	public StateMachineDeveloperEx(ReadOnlyTargetRules Target)
		: base(Target)
	{
#if UE_4_21_OR_LATER
		PrivatePCHHeaderFile = "Private/StateMachineDeveloperExPrivatePCH.h";
#endif


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
			});


		DynamicallyLoadedModuleNames.AddRange(new string[]{});
	}
}
