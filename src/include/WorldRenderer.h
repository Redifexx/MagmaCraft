#pragma once

#include <vector>

// Receives chunks from server
// Caches a number of chunks around the player
// Renders cached chunks
namespace Craft
{
	class WorldRenderer
	{
		public:

		private:
			// will determine chunk buffer size
			// will be cached from server

			uint8_t m_ChunkRenderDistance = 8;
	};
}