using UnrealBuildTool;
using System.Collections.Generic;

public class CodexTarget : TargetRules
{
	public CodexTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange(new string[] {
			"CodexCore",
			"CodexVehicleDynamics",
			"CodexVehicles",
			"CodexWorld",
			"CodexTraffic",
			"CodexPhoto",
			"CodexAudio",
			"CodexGameplay",
			"CodexUI"
		});
	}
}
