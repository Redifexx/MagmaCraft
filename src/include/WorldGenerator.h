#pragma once

#include "FastNoiseLite.h"
#include "Chunk.h"

// Generates Chunks and Writes them to Files
namespace Craft
{
	class WorldGenerator
	{
		public:
			WorldGenerator(int seed);
			void GenerateChunk(Chunk& chunk, int chunkX, int chunkZ);
		private:
			FastNoiseLite m_Noise;
	};
}