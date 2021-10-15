using System.IO;
using UnrealBuildTool;



public class StateMachineEx : ModuleRules
{
	public StateMachineEx(ReadOnlyTargetRules Target)
		: base(Target)
	{
#if UE_4_21_OR_LATER
#if UE_4_24_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V2;
#endif
		PrivatePCHHeaderFile = "Private/StateMachineExPrivatePCH.h";
#endif

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
			});

		PublicDependencyModuleNames.AddRange(new string[] {
			});


		PrivateDependencyModuleNames.AddRange(new string[] {
			});

		PrivateDependencyModuleNames.AddRange(new string[] {
			});


		DynamicallyLoadedModuleNames.AddRange(new string[]{});
	}
}
