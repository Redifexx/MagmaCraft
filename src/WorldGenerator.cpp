#include "WorldGenerator.h"
#include "Chunk.h"
#include <algorithm>

using namespace Craft;

void Spline::AddPoint(float value, float result)
{
	m_Points.push_back({ value, result });

	// sort by input value
	std::sort(m_Points.begin(), m_Points.end(), [](const SplinePoint& a, const SplinePoint& b)
	{
		return a.value < b.value;
	});
}

float Spline::Get(float value)
{
	if (m_Points.empty()) { return 0.0f; }
	if (value <= m_Points.front().value) { return m_Points.front().result; }
	if (value >= m_Points.back().value) { return m_Points.back().result; }

	for (size_t i = 0; i < m_Points.size() - 1; i++)
	{
		if (value >= m_Points[i].value && value <= m_Points[i + 1].value)
		{
			// lerp
			float t = (value - m_Points[i].value) / (m_Points[i + 1].value - m_Points[i].value);
			return m_Points[i].result + t * (m_Points[i + 1].result - m_Points[i].result);
		}
	}
	return 0.0f;
}

WorldGenerator::WorldGenerator(int seed)
{
	// once i add more options beyond seed, i must save the settings to world file header
	m_DensityNoise.SetSeed(seed);
	m_DensityNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	m_DensityNoise.SetFrequency(0.01f);
	m_DensityNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
	m_DensityNoise.SetFractalOctaves(2);

	m_Continentalness.SetSeed(seed);
	m_Continentalness.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2S);
	m_Continentalness.SetFrequency(0.003f);
	m_Continentalness.SetFractalType(FastNoiseLite::FractalType_FBm);
	m_Continentalness.SetFractalOctaves(4);
	m_Continentalness.SetFractalLacunarity(2.0f);
	m_Continentalness.SetFractalGain(0.3f);
	m_Continentalness.SetFractalWeightedStrength(-1.380f);

	m_Erosion.SetSeed(seed);
	m_Erosion.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	m_Erosion.SetFrequency(0.002f);
	m_Erosion.SetFractalType(FastNoiseLite::FractalType_FBm);
	m_Erosion.SetFractalOctaves(2);
	m_Erosion.SetFractalLacunarity(2.0f);
	m_Erosion.SetFractalGain(5.0f);
	m_Erosion.SetFractalWeightedStrength(-1.0f);

	m_PV.SetSeed(seed);
	m_PV.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2S);
	m_PV.SetFrequency(0.02f);
	m_PV.SetFractalType(FastNoiseLite::FractalType_PingPong);
	m_PV.SetFractalOctaves(2);
	m_PV.SetFractalLacunarity(0.0f);
	m_PV.SetFractalGain(0.0f);
	m_PV.SetFractalPingPongStrength(2.0);

	// define splines - tweaks world gen outcome
	m_ContinentalnessHeightSpline.AddPoint(-1.0f, 30.0f); // ocean
	m_ContinentalnessHeightSpline.AddPoint(-0.2f, 55.0f); // shoreline
	m_ContinentalnessHeightSpline.AddPoint(0.0f, 70.0f); // plains
	m_ContinentalnessHeightSpline.AddPoint(0.5f, 100.0f); // highlands
	m_ContinentalnessHeightSpline.AddPoint(1.0f, 180.0f); // big mountains

	m_ErosionMultiplierSpline.AddPoint(-1.0f, 0.0f); // flat
	m_ErosionMultiplierSpline.AddPoint(-0.4f, 5.0f); // some hills
	m_ErosionMultiplierSpline.AddPoint(0.2f, 20.0f); // rough 
	m_ErosionMultiplierSpline.AddPoint(1.0f, 60.0f); // spiky

}

void WorldGenerator::GenerateChunk(Chunk& chunk, int chunkX, int chunkY, int chunkZ)
{
	int startX = chunkX * CHUNK_WIDTH;
	int startY = chunkY * CHUNK_HEIGHT;
	int startZ = chunkZ * CHUNK_WIDTH;
	
	// NEW
	
	for (int x = 0; x < CHUNK_WIDTH; x++)
	{
		for (int z = 0; z < CHUNK_WIDTH; z++)
		{
			float globalX = startX + x;
			float globalZ = startZ + z;

			// sample from 2d noise maps using x, z
			float cont = m_Continentalness.GetNoise(globalX, globalZ);
			float erosion = m_Erosion.GetNoise(globalX, globalZ);
			float peakval = m_PV.GetNoise(globalX, globalZ);

			// determine starting height and detail/erosion stength
			float baseHeight = m_ContinentalnessHeightSpline.Get(cont);
			float detailMagnitude = m_ErosionMultiplierSpline.Get(erosion);

			// combine the values to get a target height
			float targetHeight = baseHeight + (peakval * detailMagnitude);

			for (int y = 0; y < CHUNK_WIDTH; y++)
			{
				// Global Coords
				float globalY = (float)startY + y;
				BlockID currentBlock = 0; // Air by default

				// density calc using 3d noise
				// target height would center it at 0
				float baseDensity = (targetHeight - globalY);

				float noise3D = m_DensityNoise.GetNoise(globalX, globalY, globalZ) * 20.0f; 

				float finalDensity = baseDensity + noise3D;

				if (finalDensity > 0.0f)
				{
					// biome logic
					// for now, sand when high erosion
					if (erosion > 0.3f && globalY < 75.0f)
					{
						currentBlock = 5; // sand
					}
					else if (globalY < 60.0f)
					{
						currentBlock = 1; // stone
					}
					else
					{
						currentBlock = 4; // grass
					}
				}
				else if (globalY < 64.0f)
				{
					currentBlock = 19; // water :)
				}

				if (globalY < -512) { currentBlock = 0; } // hard cutoff beneath 512
				chunk.SetBlock(x, y, z, currentBlock);
			}
		}
	}
	
}