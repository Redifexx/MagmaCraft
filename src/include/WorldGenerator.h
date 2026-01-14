#pragma once

#include "FastNoiseLite.h"

namespace Craft
{
	class WorldGenerator
	{
		public:
			WorldGenerator(int seed);
			void GenerateChunk(Chunk& chunk, int chunkX, chunkZ);
		private:
			FastNoiseLite m_Noise;
	};
}