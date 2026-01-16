#pragma once

#include <string>
#include <vector>
#include "Chunk.h"
#include "WorldGenerator.h"
#include <glm/glm.hpp>
#include <cstdint>

// Manages world data, including loading, saving, and updating chunks
namespace Craft
{
	struct ChunkFileHeader
	{
		uint32_t magic = 0X4D43484B; // Magma Chunk 'MCHK' Magic Number
		int chunkX;
		int chunkZ;
	};

	class WorldManager
	{
		public:
			WorldManager();

			// Sets up a new world generator & world folder
			void CreateWorld(const std::string& worldName, int seed);

			// Generates initial world data around player spawn
			void InitializeWorld(glm::vec3 spawnPoint);

			void CompressChunk(const Chunk& chunk, std::vector<uint_8>& compressedData);
			void DecompressChunk(const std::vector<uint8_t>& compressedData, Chunk& chunk);
			
			std::vector<uint8_t> GetChunkCompressed(int chunkX, int chunkZ);

			void SaveChunkToFile(const Chunk& chunk, int chunkX, int chunkZ);

			bool LoadChunkFromFile(Chunk& chunk, int chunkX, int chunkZ);
			bool LoadChunkFromFileCompressed(std::vector<uint8_t>& compressedData, int chunkX, int chunkZ);

			// player class to later be passed in
			void UpdateWorldAroundPlayer(const glm::vec3& playerPosition);
			
		private:
			WorldGenerator* m_WorldGenerator = nullptr;
			std::string m_WorldName = "New World";
	};
}