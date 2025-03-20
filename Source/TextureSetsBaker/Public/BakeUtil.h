// Copyright (c) 2025 Electronic Arts. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RawMesh.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTextureSetBake, Log, All);

class FBakeUtil
{
public:
	struct FBakeArgs
	{
		FRawMesh RawMesh;
		int BakeWidth = 1;
		int BakeHeight = 1;
	};

	struct FBakeResults
	{
		TArray<float> Pixels;
	};

	static void Bake(const FBakeArgs& Args, FBakeResults& Results);
	static void BakeUV(const FBakeArgs& Args, FBakeResults& Results);
	static void DilateUVs(const FBakeArgs& Args, FBakeResults& Results, int Iterations);
	static void ReplaceNANValues(FBakeResults& Results, float NewValue);

	static inline FIntVector2 IndexToCoordinate(int Index, const FBakeArgs& Args)
	{
		const int ClampedIndex = FMath::Clamp(Index, 0, Args.BakeWidth * Args.BakeHeight - 1);
		return FIntVector2(ClampedIndex % Args.BakeWidth, ClampedIndex / Args.BakeWidth);
	}

	static inline int CoordinateToIndex(FIntVector2 Coordinate, const FBakeArgs& Args)
	{
		FIntVector2 ClampedCoord = Coordinate;

		ClampedCoord.X = FMath::Clamp(Coordinate.X, 0, Args.BakeWidth - 1);
		ClampedCoord.Y = FMath::Clamp(Coordinate.Y, 0, Args.BakeHeight - 1);

		return ClampedCoord.X + (ClampedCoord.Y * Args.BakeWidth);
	}
};