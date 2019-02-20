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
				"StateMachineDeveloperEx/Public"
			});


		PrivateIncludePaths.AddRange(new string[] {
				"StateMachineDeveloperEx/Private",
			});


		PublicDependencyModuleNames.AddRange(new string[] {
				"BlueprintGraph",
				"Core",
				"UnrealEd",
			});

		PublicDependencyModuleNames.AddRange(new string[] {
			});


		PrivateDependencyModuleNames.AddRange(new string[] {
				"CoreUObject",
				"Engine",
				"KismetCompiler",
			});

		PrivateDependencyModuleNames.AddRange(new string[] {
				"StateMachineEx",
			});


		DynamicallyLoadedModuleNames.AddRange(new string[]{});
	}
}
