#include "BlockLibrary.h"

using namespace Craft;

std::vector<BlockData> BlockLibrary::blockTypes;
const float BlockLibrary::ATLAS_SIZE = 512.0f;
const float BlockLibrary::TILE_SIZE = 32.0f;
const int BlockLibrary::TILES_PER_ROW = 10;
const float BlockLibrary::TILE_PADDING = 16.0f;
float BlockLibrary::m_UVTileScale = BlockLibrary::TILE_SIZE / BlockLibrary::ATLAS_SIZE;

void BlockLibrary::Initialize()
{
	blockTypes.resize(256);

	// ID 0: Air
	blockTypes[0] = { "Air", true, false, -1, -1, -1, -1, -1, -1 };

	// ID 1: Stone
	blockTypes[1] = { "Stone", false, false, 32, 32, 32, 32, 32, 32 };

	// ID 2: Cobblestone
	blockTypes[2] = { "Cobblestone", false, false, 2, 2, 2, 2, 2, 2 };

	// ID 3: Dirt
	blockTypes[3] = { "Dirt", false, false, 16, 16, 16, 16, 16, 16 };

	// ID 4: Grass
	blockTypes[4] = { "Grass", false, false, 19, 20, 20, 20, 20, 16 };

	// ID 5: Sand
	blockTypes[5] = { "Sand", false, true, 31, 31, 31, 31, 31, 31 };

	// ID 6: Gravel
	blockTypes[6] = { "Gravel", false, true, 21, 21, 21, 21, 21, 21 };

	// ID 7: Wood
	blockTypes[7] = { "Wood", false, false, 25, 24, 24, 24, 24, 25 };

	// ID 8: Wooden Planks
	blockTypes[8] = { "WoodenPlanks", false, false, 26, 26, 26, 26, 26, 26 };

	// ID 9: Leaves
	// ID 10: Brick
	// ID 11: Bedrock
	// ID 12: Diamond Ore
}

const glm::vec2 BlockLibrary::GetTexCoords(uint8_t texID)
{
	int column = texID % TILES_PER_ROW;
	int row = texID / TILES_PER_ROW;

	float xPixel = column * (TILE_SIZE + TILE_PADDING) + TILE_PADDING;
	float yPixel = row * (TILE_SIZE + TILE_PADDING) + TILE_PADDING;


	float u = xPixel / ATLAS_SIZE;
	float v = yPixel / ATLAS_SIZE;

	// might have to handle flipped textures if not already
	return glm::vec2(u, v);
}