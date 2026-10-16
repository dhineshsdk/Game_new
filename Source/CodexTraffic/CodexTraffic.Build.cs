using UnrealBuildTool;

public class CodexTraffic : ModuleRules
{
	public CodexTraffic(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"CodexCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
		});
	}
}
