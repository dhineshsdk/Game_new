using UnrealBuildTool;

public class CodexTests : ModuleRules
{
	public CodexTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"CodexCore",
			"CodexVehicleDynamics"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
		});
	}
}
