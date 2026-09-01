#pragma once

#include "CoreMinimal.h"
#include "TdTypes.h"

/** Index math for a square heightfield. Implement neighbours here so A* stays readable. */
namespace TdGrid
{
	inline int32 Index(int32 X, int32 Y, int32 Resolution)
	{
		return Y * Resolution + X;
	}

	inline int32 Index(const FTdGridCoord& C, int32 Resolution)
	{
		return Index(C.X, C.Y, Resolution);
	}

	inline FTdGridCoord Coord(int32 FlatIndex, int32 Resolution)
	{
		return FTdGridCoord(FlatIndex % Resolution, FlatIndex / Resolution);
	}

	inline bool IsInside(int32 X, int32 Y, int32 Resolution)
	{
		return X >= 0 && Y >= 0 && X < Resolution && Y < Resolution;
	}

	inline bool IsInside(const FTdGridCoord& C, int32 Resolution)
	{
		return IsInside(C.X, C.Y, Resolution);
	}

	/** 4-connected. Cleaner roads. Prefer this for Part 1. */
	inline void Neighbours4(const FTdGridCoord& C, int32 Resolution, TArray<FTdGridCoord>& Out)
	{
		Out.Reset();
		const int32 DX[4] = { 1, -1, 0, 0 };
		const int32 DY[4] = { 0, 0, 1, -1 };
		for (int32 i = 0; i < 4; ++i)
		{
			const FTdGridCoord N(C.X + DX[i], C.Y + DY[i]);
			if (IsInside(N, Resolution))
			{
				Out.Add(N);
			}
		}
	}

	inline FVector CellCentre(const FTdGridCoord& C, float CellSize, float Height)
	{
		return FVector((C.X + 0.5f) * CellSize, (C.Y + 0.5f) * CellSize, Height);
	}
}
