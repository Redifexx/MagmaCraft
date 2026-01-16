#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Chunk.h"

// Receives chunks from server
// Caches a number of chunks around the player
// Renders cached chunks
namespace Craft
{
	class WorldRenderer
	{
		public:
			WorldRenderer();

			void RequestChunksAroundPlayer(const glm::vec3& playerPosition);

			void SetChunkRenderDistance(uint8_t distance) { m_ChunkRenderDistance = distance; }

		private:
			// will determine chunk buffer size
			// will be cached from server

			std::vector<Chunk> m_ChunkBuffer;

			uint8_t m_ChunkRenderDistance = 8;
	};
}