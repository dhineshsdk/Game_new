using UnrealBuildTool;

public class CodexPhoto : ModuleRules
{
	public CodexPhoto(ReadOnlyTargetRules Target) : base(Target)
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
