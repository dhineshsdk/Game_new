using UnrealBuildTool;

// Pure C++20 module (ADR 0005). UE Core is linked only for module registration in
// Private/CodexCoreModule.cpp. tools/check_purity.ps1 enforces the dependency list below.
public class CodexCore : ModuleRules
{
	public CodexCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;
		FPSemantics = FPSemanticsMode.Precise;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core"
		});
	}
}
