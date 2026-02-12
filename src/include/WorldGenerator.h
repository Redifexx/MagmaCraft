#pragma once

#include <FastNoiseLite.h>
#include "Chunk.h"
#include <vector>

// Generates Chunks and Writes them to Files
// Holds the "recipe" needed for Terrain Generation
namespace Craft
{
	// helpers to define spline points for noise interpolation
	struct SplinePoint
	{
		float value;
		float result;
	};

	class Spline
	{
		public:
			void AddPoint(float value, float result);
			float Get(float value);
		private:
			std::vector<SplinePoint> m_Points;
	};

	class WorldGenerator
	{
		public:
			WorldGenerator(int seed);
			void GenerateChunk(Chunk& chunk, int chunkX, int chunkY, int chunkZ);
		private:
			// thank you Henrik Kniberg
			FastNoiseLite m_DensityNoise;
			FastNoiseLite m_Continentalness;
			FastNoiseLite m_Erosion;
			FastNoiseLite m_PV;

			Spline m_ContinentalnessHeightSpline;
			Spline m_ErosionMultiplierSpline;
	};
}