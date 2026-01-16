#include "Chunk.h"

using namespace Craft;

BlockID Chunk::GetBlock(int x, int y, int z) const
{
	if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_WIDTH)
		return 0; // Air block for out of bounds

	int index = x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH);
	return blocks[index];
}

void Chunk::SetBlock(int x, int y, int z, BlockID block)
{
	if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_WIDTH)
		return; // Ignore out of bounds
	int index = x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH);
	blocks[index] = block;
}