// Copyright (c) 2025 Electronic Arts. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TextureSetsDemoTarget : TargetRules
{
	public TextureSetsDemoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("TextureSetsDemo");
	}
}
