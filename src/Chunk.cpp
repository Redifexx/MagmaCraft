#include "Chunk.h"

using namespace Craft;

BlockID Chunk::GetBlockData(int x, int y, int z) const
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
	m_IsModified = true;
}

const int32_t Chunk::GetLocalBlockNeighbor(uint32_t id, Direction direction)
{
	// turn into coords
	int x = GetBlockX(id);
	int y = GetBlockY(id);
	int z = GetBlockZ(id);

	// block offset
	switch (direction)
	{
		case (Direction::EAST):
			x++;
			break;
		case (Direction::WEST):
			x--;
			break;
		case (Direction::UP):
			y++;
			break;
		case (Direction::DOWN):
			y--;
			break;
		case (Direction::SOUTH):
			z++;
			break;
		case (Direction::NORTH):
			z--;
			break;
	}

	// check chunk bounds
	if (
		x < 0 || x >= CHUNK_WIDTH ||
		y < 0 || y >= CHUNK_HEIGHT ||
		z < 0 || z >= CHUNK_WIDTH)
	{
		return -1; // outside chunk
	}

	// else turn back into index 
	return x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH);
}