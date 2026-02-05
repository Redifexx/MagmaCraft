#pragma once

#include <cstdint>
#include <glm/glm.hpp>

// Data Representation of a chunk
namespace Craft
{
	using BlockID = uint8_t;

	enum class Direction
	{
		EAST, // +X
		WEST, // -X
		UP, // +Y
		DOWN, // -Y
		SOUTH, // +Z
		NORTH, // -Z
	};

	const int CHUNK_WIDTH = 16;
	const int CHUNK_HEIGHT = 256;
	const int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH;

	class Chunk
	{
		public:
			// Chunk data stored in a 1D array
			BlockID blocks[CHUNK_VOLUME];
			bool m_IsModified = false;

			BlockID GetBlockData(int x, int y, int z) const;
			void SetBlock(int x, int y, int z, BlockID block);

			// Turns the index of a block into local coordinates
			const int GetBlockX(uint32_t id) { return id % CHUNK_WIDTH; }
			const int GetBlockY(uint32_t id) { return id / (CHUNK_WIDTH * CHUNK_WIDTH); }
			const int GetBlockZ(uint32_t id) { return (id / CHUNK_WIDTH) % CHUNK_WIDTH; }
			glm::ivec3 GetBlockXYZ(uint32_t id) { return glm::ivec3(GetBlockX(id), GetBlockY(id), GetBlockZ(id)); }
			const int32_t GetLocalBlockNeighbor(uint32_t id, Direction direction);
	};
}