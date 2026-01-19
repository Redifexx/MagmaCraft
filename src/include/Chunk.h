#pragma once

#include <cstdint>

// Data Representation of a chunk
namespace Craft
{
	using BlockID = uint8_t;

	const int CHUNK_WIDTH = 16;
	const int CHUNK_HEIGHT = 256;
	const int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH;

	class Chunk
	{
		public:
			// Chunk data stored in a 1D array
			BlockID blocks[CHUNK_VOLUME];
			bool m_IsModified = false;

			BlockID GetBlock(int x, int y, int z) const;
			void SetBlock(int x, int y, int z, BlockID block);
	};
}