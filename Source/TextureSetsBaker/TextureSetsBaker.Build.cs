// Copyright (c) 2025 Electronic Arts. All Rights Reserved.

using UnrealBuildTool;

public class TextureSetsBaker : ModuleRules
{
	public TextureSetsBaker(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"TextureSetsCommon",
				"TextureSets",
				"TextureSetsCompiler",
				"TextureSetsMaterialBuilder",
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"MeshDescription",
				"StaticMeshDescription",
			}
			);

		AddEngineThirdPartyPrivateStaticDependencies(Target, "Embree3");
	}
}
