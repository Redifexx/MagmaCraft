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

			void CompressChunk(const Chunk& chunk, std::vector<uint8_t>& compressedData);
			void DecompressChunk(const std::vector<uint8_t>& compressedData, Chunk& chunk);
			
			std::vector<uint8_t> GetChunkCompressed(int chunkX, int chunkZ);

			void SaveChunkToFile(const Chunk& chunk, int chunkX, int chunkZ);

			bool LoadChunkFromFile(std::vector<uint8_t>& compressedData, int chunkX, int chunkZ);
			bool LoadChunkFromFileDecompressed(Chunk& chunk, int chunkX, int chunkZ);

			
			
		private:
			WorldGenerator* m_WorldGenerator = nullptr;
			std::string m_WorldName = "New World";
			std::map<std::pair<int, int>, SingleChunk> m_ChunkBuffer;
	};
}