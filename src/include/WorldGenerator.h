#pragma once

#include <FastNoiseLite.h>
#include "Chunk.h"

// Generates Chunks and Writes them to Files
// Holds the "recipe" needed for Terrain Generation
namespace Craft
{
	class WorldGenerator
	{
		public:
			WorldGenerator(int seed);
			void GenerateChunk(Chunk& chunk, int chunkX, int chunkY, int chunkZ);
		private:
			FastNoiseLite m_Noise;
			FastNoiseLite m_Continentalness;
	};
}