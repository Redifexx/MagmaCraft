#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace Craft
{
	struct BlockData
	{
		std::string name;
		bool isTransparent;
		bool hasGravity;
		int textureTop, textureSouth, textureEast, textureNorth, textureWest, textureBottom;
	};

	class BlockLibrary
	{
		std::vector<BlockData> blockTypes;

		BlockLibrary();

		const BlockData& GetBlockData(uint8_t id) { return blockTypes[id]; }
	};
}