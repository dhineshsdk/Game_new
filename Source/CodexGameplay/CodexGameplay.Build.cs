using UnrealBuildTool;

public class CodexGameplay : ModuleRules
{
	public CodexGameplay(ReadOnlyTargetRules Target) : base(Target)
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
