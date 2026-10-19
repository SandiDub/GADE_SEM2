using UnrealBuildTool;
using System.Collections.Generic;

public class GADE_SEM2Target : TargetRules
{
	public GADE_SEM2Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;

        ExtraModuleNames.AddRange( new string[] { "GADE_SEM2" } );
	}
}
