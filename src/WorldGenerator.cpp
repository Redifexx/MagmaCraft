#include "WorldGenerator.h"
#include "Chunk.h"

using namespace Craft;

WorldGenerator::WorldGenerator(int seed)
{
	m_Noise.SetSeed(seed);
	m_Noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
	m_Noise.SetFrequency(0.01f);
}

void WorldGenerator::GenerateChunk(Chunk& chunk, int chunkX, int chunkZ)
{
	int startX = chunkX * CHUNK_WIDTH;
	int startZ = chunkZ * CHUNK_WIDTH;

	for (int x = 0; x < CHUNK_WIDTH; x++)
	{
		for (int z = 0; z < CHUNK_WIDTH; z++)
		{
			// Global Coords
			float globalX = startX + x;
			float globalZ = startZ + z;
			
			// Get height from noise
			float noiseValue = m_Noise.GetNoise(globalX, globalZ);
			int height = 64 + (int)(noiseValue * 20); // Base height 64 with 20 block variation

			for (int y = 0; y < CHUNK_HEIGHT; y++)
			{
				BlockID currentBlock = 0; // Air by default

				if (y < height - 4)
				{
					currentBlock = 1; // Stone
				}
				else if (y < height)
				{
					currentBlock = 5; // Sand
				}
				else if (y == height)
				{
					currentBlock = 3; // Dirt
				}

				chunk.SetBlock(x, y, z, currentBlock);
			}
		}
	}
}