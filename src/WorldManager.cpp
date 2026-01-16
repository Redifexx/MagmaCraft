#include "WorldManager.h"
#include <glm/glm.hpp>

using namespace Craft;

WorldManager::WorldManager()
{
}

void WorldManager::CreateWorld(const std::string& worldName, int seed)
{
	// Implementation for creating a new world with the given name and seed
	m_WorldGenerator = new WorldGenerator(seed);

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
