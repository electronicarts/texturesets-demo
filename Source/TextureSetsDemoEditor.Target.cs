// Copyright (c) 2025 Electronic Arts. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TextureSetsDemoEditorTarget : TargetRules
{
	public TextureSetsDemoEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("TextureSetsDemo");
	}
}
