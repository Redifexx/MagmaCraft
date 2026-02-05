#include "WorldManager.h"
#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include "NetworkManager.h"
#include "Core/Texture.h"

#undef min

using namespace Craft;
 
void WorldManager::CreateWorld(std::string& worldName, int seed, EntityWorld& eWorld)
{
	// Implementation for creating a new world with the given name and seed
	m_WorldGenerator = std::make_unique<WorldGenerator>(seed);

	// truncate world name, replace spaces with gap
	size_t maxNameLength = 32;
	worldName.resize(std::min(worldName.size(), maxNameLength));
	std::replace(worldName.begin(), worldName.end(), ' ', '_');

	m_WorldName = worldName;

	// Create .mcwd file (magmacraftworld)
	std::string folderPath = "saves/" + m_WorldName;
	std::filesystem::create_directories(folderPath);

	std::string filename = folderPath + "/" + m_WorldName + ".mcwd";

	std::ofstream outfile(filename, std::ios::binary);
	if (!outfile.is_open()) return; // should prob throw an error

	// Header
	WorldFileHeader header;
	header.seed = seed;

	// 1. Zero out the memory first so there is no garbage
	std::memset(header.worldName, 0, sizeof(header.worldName));

	// 2. Copy the actual string characters into the array
	// We use std::min to ensure we don't overflow the 32 byte buffer
	size_t copyLen = std::min(m_WorldName.size(), sizeof(header.worldName) - 1);
	std::memcpy(header.worldName, m_WorldName.c_str(), copyLen);

	// the header contains all the data for now until the world needs more
	outfile.write((char*)&header, sizeof(WorldFileHeader));

	outfile.close();

	// later make player spawn random within chunk
	InitializeWorld(glm::vec3(0.0f, 64.0f, 0.0f));
}

bool WorldManager::LoadWorld(const char* filepath)
{
	// looks for worldname.mcwd
	
	std::ifstream infile(filepath, std::ios::binary);
	if (!infile.is_open()) return false;

	// Read Header
	WorldFileHeader header;
	infile.read((char*)&header, sizeof(WorldFileHeader));

	if (header.magic != 0X4D435744)
	{
		infile.close();
		return false; // Invalid file
	}
	infile.close();

	// creates world generator with the same attributes
	// sets the world name
	m_WorldGenerator = std::make_unique<WorldGenerator>(header.seed);
	m_WorldName = header.worldName;

	return true;
}

void WorldManager::SaveWorld(EntityWorld& eWorld)
{
	auto playerEntities = eWorld.View<PlayerComponent>();
	for (uint32_t entity : playerEntities)
	{
		SavePlayerData(eWorld, entity);
	}
}

std::string WorldManager::GetPlayerFolder()
{
	return "saves/" + m_WorldName + "/players/";
}

void WorldManager::SavePlayerData(EntityWorld& eWorld, uint32_t entityID)
{
	auto& playerRef = eWorld.GetComponent<PlayerComponent>(entityID);
	auto& transformRef = eWorld.GetComponent<TransformComponent>(entityID);
	auto& healthRef = eWorld.GetComponent<HealthComponent>(entityID);
	auto& physicsRef = eWorld.GetComponent<PhysicsComponent>(entityID);

	auto idMap = m_NetworkIDToNameMap.lock();
	if (!idMap) return;

	std::string username = (*idMap)[playerRef.networkID];


	SerializedPlayerData pData = {};

	pData.posX = transformRef.localPosition.x;
	pData.posY = transformRef.localPosition.y;
	pData.posZ = transformRef.localPosition.z;

	pData.rotW = transformRef.localRotation.w;
	pData.rotX = transformRef.localRotation.x;
	pData.rotY = transformRef.localRotation.y;
	pData.rotZ = transformRef.localRotation.z;

	pData.health = healthRef.health;

	pData.velX = physicsRef.velocity.x;
	pData.velY = physicsRef.velocity.y;
	pData.velZ = physicsRef.velocity.z;

	std::cout << "Saving data for " << username << std::endl;
	std::cout << "Leaving at " << pData.posX;
	std::cout << " " << pData.posY;
	std::cout << " " << pData.posZ << std::endl;

	// write to file
	std::filesystem::create_directories(GetPlayerFolder());
	std::string filename = GetPlayerFolder() + std::string(username) + ".mcpl";
	std::ofstream outfile(filename, std::ios::binary);
	if (!outfile.is_open()) return; // should prob throw an error

	PlayerFileHeader header; // just a magic number for now
	outfile.write((char*)&header, sizeof(PlayerFileHeader));

	outfile.write((char*)&pData, sizeof(SerializedPlayerData));

	outfile.close();
}

bool WorldManager::LoadPlayerData(const std::string& username, SerializedPlayerData& outData)
{
	std::string filename = GetPlayerFolder() + username + ".mcpl";

	std::ifstream infile(filename, std::ios::binary);
	if (!infile.is_open()) return false; // new player

	PlayerFileHeader header; // just a magic number for now
	infile.read((char*)&header, sizeof(PlayerFileHeader));

	if (header.magic != 0X4D43504C)
	{
		infile.close();
		return false; // Invalid file
	}

	infile.read((char*)(&outData), sizeof(SerializedPlayerData));
	return true;
}

uint32_t WorldManager::CreatePlayerEntity(EntityWorld& eWorld, uint8_t networkID)
{
	uint32_t playerEntity = eWorld.AddEntity();

	auto idMap = m_NetworkIDToNameMap.lock();
	if (!idMap) return NULL_ENTITY;

	std::string username = (*idMap)[networkID];

	SerializedPlayerData pData = {};
	bool isSaved = LoadPlayerData(username, pData);

	// later add randomized spawn based on world placement
	glm::vec3 spawnPosition = glm::vec3(0.0f, 64.0f, 0.0f);
	glm::quat spawnRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	float spawnHealth = 20.0f;

	if (isSaved)
	{
		std::cout << username << "'s data loaded successfully!" << std::endl;
		std::cout << "Joining at " << pData.posX;
		std::cout << " " << pData.posY;
		std::cout << " " << pData.posZ << std::endl;
		spawnPosition = glm::vec3(pData.posX, pData.posY, pData.posZ);
		spawnRotation = glm::quat(pData.rotW, pData.rotX, pData.rotY, pData.rotZ);
		spawnHealth = pData.health;
	}

	// Add Components (Local + Remote)

	Magma::Texture* skin = new Magma::Texture("resources/textures/player_skin.png");
	skin->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	skin->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	eWorld.AddComponent<TransformComponent>(playerEntity, { spawnPosition, spawnRotation, glm::vec3(1.0f) }); // scale down for model testing
	
	eWorld.AddComponent<PlayerComponent>(playerEntity, {
		networkID,
		false,
		0,
		std::move(skin) // ensure no mem leak on destruction
	});

	//eWorld.AddComponent<PlayerComponent>(playerEntity, {
	//	networkID
	//});

	eWorld.AddComponent<HealthComponent>(playerEntity, { spawnHealth, 20.0f });
	eWorld.AddComponent<PhysicsComponent>(playerEntity, { glm::vec3(0.0f), true });
	eWorld.AddComponent<RelationshipComponent>(playerEntity,
	{ 
		Craft::NULL_ENTITY,
		Craft::NULL_ENTITY,
		Craft::NULL_ENTITY,
		Craft::NULL_ENTITY
	});

	//safety
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// will render unless ur a local player
	Magma::Model* model = new Magma::Model("resources/models/player.fbx");
	eWorld.AddComponent<ModelComponent>(playerEntity, { std::move(model) });

	return playerEntity;
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
	std::string filename = folderPath + "/chunk_" + std::to_string(chunkX) + "_" + std::to_string(chunkZ) + ".dat";

	std::ofstream outfile(filename, std::ios::binary);
	if (!outfile.is_open()) return; // should prob throw an error

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

	Chunk* chunk = GetChunkFromBuffer(chunkX, chunkZ);

	if (chunk->m_IsModified)
	{
		SaveChunkToFile(*chunk, chunkX, chunkZ);
	}

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