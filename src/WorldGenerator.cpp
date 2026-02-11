#include "WorldGenerator.h"
#include "Chunk.h"

using namespace Craft;

WorldGenerator::WorldGenerator(int seed)
{
	// once i add more options beyond seed, i must save the settings to world file header
	m_Noise.SetSeed(seed);
	m_Noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	m_Noise.SetFrequency(0.01f);
	m_Noise.SetFractalType(FastNoiseLite::FractalType_FBm);
	m_Noise.SetFractalOctaves(4);
}

void WorldGenerator::GenerateChunk(Chunk& chunk, int chunkX, int chunkY, int chunkZ)
{
	int startX = chunkX * CHUNK_WIDTH;
	int startY = chunkY * CHUNK_HEIGHT;
	int startZ = chunkZ * CHUNK_WIDTH;
	
	// NEW
	
	for (int x = 0; x < CHUNK_WIDTH; x++)
	{
		for (int y = 0; y < CHUNK_WIDTH; y++)
		{
			for (int z = 0; z < CHUNK_WIDTH; z++)
			{
				// Global Coords
				float globalX = startX + x;
				float globalY = startY + y;
				float globalZ = startZ + z;

				BlockID currentBlock = 0; // Air by default

				// Get height from noise
				float noiseValue = m_Noise.GetNoise(globalX, globalY, globalZ);
				if (noiseValue > 0.0)
				{
					currentBlock = 2;
				}
				chunk.SetBlock(x, y, z, currentBlock);
			}
		}
	}
	
}