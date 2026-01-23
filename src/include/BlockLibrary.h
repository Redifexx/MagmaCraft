#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <glm/glm.hpp>

namespace Craft
{
	struct BlockData
	{
		std::string name = "null";
		bool isTransparent = true;
		bool hasGravity = true;
		int textureTop = 0;
		int textureSouth = 0;
		int textureEast = 0;
		int textureNorth = 0;
		int textureWest = 0;
		int textureBottom = 0;
	};

	class BlockLibrary
	{
		public:

			static std::vector<BlockData> blockTypes;
			static void Initialize();
			static const BlockData& GetBlockData(uint8_t id) { return blockTypes[id]; }

			// Returns texture coordinates from the texture atlas, assuming 256x256

			static const float ATLAS_SIZE;
			static const float TILE_SIZE;
			static const int TILES_PER_ROW;
			static const float PADDING;
			static float m_UVTileScale;
			static const glm::vec2 GetTexCoords(uint8_t texID);
	};
}