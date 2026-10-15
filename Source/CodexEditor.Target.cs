using UnrealBuildTool;
using System.Collections.Generic;

public class CodexEditorTarget : TargetRules
{
	public CodexEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;
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
			"CodexUI",
			"CodexEditor",
			"CodexTests"
		});
	}
}
