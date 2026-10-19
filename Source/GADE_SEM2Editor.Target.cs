using UnrealBuildTool;
using System.Collections.Generic;

public class GADE_SEM2EditorTarget : TargetRules
{
	public GADE_SEM2EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        bOverrideBuildEnvironment = true;

        ExtraModuleNames.AddRange( new string[] { "GADE_SEM2" } );
	}
}
