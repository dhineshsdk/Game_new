using UnrealBuildTool;

public class CodexAudio : ModuleRules
{
	public CodexAudio(ReadOnlyTargetRules Target) : base(Target)
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
