using UnrealBuildTool;

public class CodexVehicles : ModuleRules
{
	public CodexVehicles(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"PhysicsCore",
			"Chaos",
			"CodexCore",
			"CodexVehicleDynamics"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
		});
	}
}
