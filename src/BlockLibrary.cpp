#include "BlockLibrary.h"

using namespace Craft;

void BlockLibrary::Initialize()
{
	blockTypes.resize(256);

	// ID 0: Air
	blockTypes[0] = { "Air", true, false, -1, -1, -1, -1, -1, -1 };
	// ID 1: Stone
	blockTypes[1] = { "Stone", false, false, 1, 1, 1, 1, 1, 1 };
	// ID 2: Cobblestone
	blockTypes[2] = { "Cobblestone", false, false, 16, 16, 16, 16, 16, 16 };
	// ID 3: Dirt
	blockTypes[3] = { "Dirt", false, false, 2, 2, 2, 2, 2, 2 };
	// ID 4: Grass
	blockTypes[4] = { "Grass", false, false, 0, 0, 0, 0, 0, 0 };
	// ID 5: Sand
	blockTypes[5] = { "Sand", false, true, 18, 18, 18, 18, 18, 18 };
	// ID 6: Gravel
	blockTypes[6] = { "Gravel", false, true, 19, 19, 19, 19, 19, 19 };
	// ID 7: Wood
	blockTypes[7] = { "Wood", false, false, 20, 21, 21, 21, 21, 20 };
	// ID 8: Wooden Planks
	blockTypes[8] = { "WoodenPlanks", false, false, 4, 4, 4, 4, 4, 4 };

	// ID 9: Leaves
	// ID 10: Brick
	// ID 11: Bedrock
	// ID 12: Diamond Ore
}

static const glm::vec2 GetTexCoords(uint8_t texID)
{
	float atlasSize = 256.0f; // 256 x 256 texture
	float tileSize = 16.0f;
	int tilesPerRow = 16; // in case it changes
	float tileUVSize = 1.0f / (float)tilesPerRow;

	int column = texID % tilesPerRow;
	int row = texID / tilesPerRow;

	float u = column * tileUVSize;
	float v = row * tileUVSize;

	// might have to handle flipped textures if not already
	return glm::vec2(u, v);
}