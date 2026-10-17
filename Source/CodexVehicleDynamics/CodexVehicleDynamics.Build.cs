using UnrealBuildTool;

// Pure C++20 module (ADR 0005). UE Core is linked only for module registration in
// Private/CodexVehicleDynamicsModule.cpp. tools/check_purity.ps1 enforces the dependency list below.
public class CodexVehicleDynamics : ModuleRules
{
	public CodexVehicleDynamics(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;
		FPSemantics = FPSemanticsMode.Precise;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CodexCore"
		});
	}
}
