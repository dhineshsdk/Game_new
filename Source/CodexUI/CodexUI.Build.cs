using UnrealBuildTool;

public class CodexUI : ModuleRules
{
	public CodexUI(ReadOnlyTargetRules Target) : base(Target)
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
