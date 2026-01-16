#include "WorldManager.h"
#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <fstream>

using namespace Craft;

WorldManager::WorldManager()
{
}

void WorldManager::CreateWorld(const std::string& worldName, int seed)
{
	// Implementation for creating a new world with the given name and seed
	m_WorldGenerator = new WorldGenerator(seed);

	m_WorldName = worldName;

	// later make player spawn random within chunk
	InitializeWorld(glm::vec3(0.0f, 64.0f, 0.0f));
}

void WorldManager::InitializeWorld(glm::vec3 spawnPoint)
{
	// Implementation for generating initial world data around player spawn
	int spawnChunkX = static_cast<int>(spawnPoint.x) / CHUNK_WIDTH;
	int spawnChunkZ = static_cast<int>(spawnPoint.z) / CHUNK_WIDTH;

	// Generate chunks around spawn point
	for (int x = -2; x <= 2; x++)
	{
		for (int z = -2; z <= 2; z++)
		{
			Chunk chunk;
			m_WorldGenerator->GenerateChunk(chunk, spawnChunkX + x, spawnChunkZ + z);

			// Save or store the generated chunk as needed

		}
	}
}

std::vector<uint8_t> WorldManager::CompressChunk(const Chunk& chunk)
{
	std::vector<uint8_t> buffer;

	// interate through all blocks in array
	for (int i = 0; i < CHUNK_VOLUME; ++i)
	{
		BlockID currentBlock = chunk.blocks[i];
		int runLength = 1;

		// Run-Length Encoding
		while ((i + 1 < CHUNK_VOLUME) && (chunk.blocks[i + 1] == currentBlock) && (runLength < 255))
		{
			runLength++;
			i++;
		}

		// Write block ID and run length to buffer
		// [Count, BlockID]
		buffer.push_back(static_cast<uint8_t>(runLength));
		buffer.push_back(static_cast<uint8_t>(currentBlock));
	}

	return buffer;
}

// Each Chunk will be its own file until worlds become bigger
void WorldManager::SaveChunkToFile(const Chunk& chunk, int chunkX, int chunkZ)
{
	std::string filename = "saves/" + m_WorldName + "/chunk_" + std::to_string(chunkX) + "_" + std::to_string(chunkZ) + ".dat";

	std::ofstream outfile(filename, std::ios::binary);
	if (!outfile.is_open()) return;

	// Header
	ChunkFileHeader header;
	header.chunkX = chunkX;
	header.chunkZ = chunkZ;
	outfile.write((char*)&header, sizeof(ChunkFileHeader));

	// Compress
	std::vector<uint8_t> compressedData = CompressChunk(chunk);

	// Write Data Size
	uint32_t dataSize = compressedData.size();
	outfile.write((char*)&dataSize, sizeof(uint32_t));

	// Write Data
	outfile.write((char*)compressedData.data(), dataSize);

	outfile.close();
}
