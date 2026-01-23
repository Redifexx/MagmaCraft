#include "WorldManager.h"
#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <fstream>
#include <filesystem>

using namespace Craft;

void WorldManager::CreateWorld(const std::string& worldName, int seed)
{
	// Implementation for creating a new world with the given name and seed
	m_WorldGenerator = std::make_unique<WorldGenerator>(seed);

	m_WorldName = worldName;

	// later make player spawn random within chunk
	InitializeWorld(glm::vec3(0.0f, 64.0f, 0.0f));
}

void WorldManager::InitializeWorld(glm::vec3 spawnPoint)
{
	// Implementation for generating initial world data around player spawn
	int spawnChunkX = WorldToChunkPos(static_cast<int>(spawnPoint.x));
	int spawnChunkZ = WorldToChunkPos(static_cast<int>(spawnPoint.z));

	// Generate chunks around spawn point
	for (int x = -2; x <= 2; x++)
	{
		for (int z = -2; z <= 2; z++)
		{
			std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>();
			CreateChunk(*chunk, spawnChunkX + x, spawnChunkZ + z);
		}
	}
}

void WorldManager::CompressChunkData(const Chunk& chunk, std::vector<uint8_t>& compressedData)
{
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
		compressedData.push_back(static_cast<uint8_t>(runLength));
		compressedData.push_back(static_cast<uint8_t>(currentBlock));
	}
}

void WorldManager::DecompressChunkData(const std::vector<uint8_t>& compressedData, Chunk& chunk)
{
	int dataIndex = 0;
	int blockIndex = 0;
	while (dataIndex < compressedData.size() && blockIndex < CHUNK_VOLUME)
	{
		uint8_t runLength = compressedData[dataIndex++];
		uint8_t blockID = compressedData[dataIndex++];
		for (int i = 0; i < runLength; ++i)
		{
			if (blockIndex < CHUNK_VOLUME)
			{
				chunk.blocks[blockIndex++] = blockID;
			}
		}
	}
}

std::vector<uint8_t> WorldManager::GetChunkDataCompressed(int chunkX, int chunkZ)
{
	std::vector<uint8_t> chunkData;
	if (LoadChunkFromFile(chunkData, chunkX, chunkZ)) return chunkData;

	// if chunk not found, generate new chunk
	Chunk chunk;
	m_WorldGenerator->GenerateChunk(chunk, chunkX, chunkZ);
	std::vector<uint8_t> buffer;
	CompressChunkData(chunk, buffer);
	return buffer;
}

void WorldManager::CreateChunk(Chunk& chunk, int chunkX, int chunkZ)
{
	m_WorldGenerator->GenerateChunk(chunk, chunkX, chunkZ);
	SaveChunkToFile(chunk, chunkX, chunkZ);
}

// Each Chunk will be its own file until worlds become bigger
void WorldManager::SaveChunkToFile(const Chunk& chunk, int chunkX, int chunkZ)
{

	std::string folderPath = "saves/" + m_WorldName;
	std::filesystem::create_directories(folderPath);
	std::string filename = folderPath + "/chunk_" + std::to_string(chunkX) + "_" + std::to_string(chunkZ) + ".dat";

	std::ofstream outfile(filename, std::ios::binary);
	if (!outfile.is_open()) return;

	// Header
	ChunkFileHeader header;
	header.chunkX = chunkX;
	header.chunkZ = chunkZ;
	outfile.write((char*)&header, sizeof(ChunkFileHeader));

	// Compress
	std::vector<uint8_t> compressedData;
	CompressChunkData(chunk, compressedData);

	// Write Data Size
	uint32_t dataSize = compressedData.size();
	outfile.write((char*)&dataSize, sizeof(uint32_t));

	// Write Data
	outfile.write((char*)compressedData.data(), dataSize);

	outfile.close();
}

bool WorldManager::LoadChunkFromFile(std::vector<uint8_t>& compressedData, int chunkX, int chunkZ)
{
	std::string filename = "saves/" + m_WorldName + "/chunk_" + std::to_string(chunkX) + "_" + std::to_string(chunkZ) + ".dat";
	std::ifstream infile(filename, std::ios::binary);
	if (!infile.is_open()) return false;

	// Read Header
	ChunkFileHeader header;
	infile.read((char*)&header, sizeof(ChunkFileHeader));
	if (header.magic != 0X4D43484B)
	{
		infile.close();
		return false; // Invalid file
	}

	// Read Data Size
	uint32_t dataSize;
	infile.read((char*)&dataSize, sizeof(uint32_t));

	// Read Compressed Data
	compressedData.resize(dataSize);
	infile.read((char*)compressedData.data(), dataSize);
	infile.close();

	return true;
}

bool WorldManager::LoadChunkFromFileDecompressed(Chunk& chunk, int chunkX, int chunkZ)
{
	std::vector<uint8_t> compressedData;
	if (!LoadChunkFromFile(compressedData, chunkX, chunkZ)) return false;

	DecompressChunkData(compressedData, chunk);
	return true;
}

void WorldManager::AddChunkToBuffer(int chunkX, int chunkZ)
{
	// First check if it's in buffer
	if (HasChunkInBuffer(chunkX, chunkZ)) return;

	// Then try to load from file
	std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>();
	if (!LoadChunkFromFileDecompressed(*chunk, chunkX, chunkZ))
	{
		// if it fails, generate a new chunk
		// first saves to file, then saves to buffer
		CreateChunk(*chunk, chunkX, chunkZ);
	}

	m_ChunkBuffer[{chunkX, chunkZ}] = std::move(chunk);
}

void WorldManager::RemoveChunkFromBuffer(int chunkX, int chunkZ)
{
	// First check if it's in buffer
	if (!HasChunkInBuffer(chunkX, chunkZ)) return;

	m_ChunkBuffer.erase({ chunkX, chunkZ });
}

void WorldManager::AddChunkDataToBuffer(std::unique_ptr<Chunk> chunk, int chunkX, int chunkZ)
{
	// overwrite
	if (HasChunkInBuffer(chunkX, chunkZ))
	{
		m_ChunkBuffer.erase({ chunkX, chunkZ });
	}

    m_ChunkBuffer[{ chunkX, chunkZ }] = std::move(chunk);
}

const uint32_t WorldManager::GetBlockNeighborData(uint32_t id, Chunk* chunk, glm::ivec2 chunkPos, Direction direction)
{
	// First check if neighbor is within chunk
	int32_t blockNeighbor = chunk->GetLocalBlockNeighbor(id, direction);
	if (blockNeighbor > -1) return chunk->blocks[blockNeighbor];

	// if out of bounds, get adjacent chunk
	// not handling up and down
	glm::ivec3 localBlockCoords = chunk->GetBlockXYZ(id);
	glm::ivec2 adjacentChunkPos = chunkPos;

	int newLocalX = localBlockCoords.x;
	int newLocalZ = localBlockCoords.z;
	switch (direction)
	{
		case (Direction::EAST):
			adjacentChunkPos.x++;
			newLocalX = 0; // 15 -> 0
			break;
		case (Direction::WEST):
			adjacentChunkPos.x--;
			newLocalX = CHUNK_WIDTH - 1; // 0 <- 15
			break;
		case (Direction::SOUTH):
			adjacentChunkPos.y++;
			newLocalZ = 0;
			break;
		case (Direction::NORTH):
			adjacentChunkPos.y--;
			newLocalZ = CHUNK_WIDTH - 1;
			break;
		default:
			return 0;
	}

	// may cause a problem as client
	if (!HasChunkInBuffer(adjacentChunkPos.x, adjacentChunkPos.y)) return 0;

	// expensive, possible optimization by caching these chunks
	Chunk* adjacentChunk = GetChunkFromBuffer(adjacentChunkPos.x, adjacentChunkPos.y);
	return adjacentChunk->GetBlockData(newLocalX, localBlockCoords.y, newLocalZ);
}