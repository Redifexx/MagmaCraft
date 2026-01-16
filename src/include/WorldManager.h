#pragma once

#include <string>
#include "Chunk.h"
#include "WorldGenerator.h"
#include <glm/glm.hpp>

// Manages world data, including loading, saving, and updating chunks
namespace Craft
{
	class WorldManager
	{
		public:
			WorldManager();

			// Sets up a new world generator & world folder
			void CreateWorld(const std::string& worldName, int seed);

			// Generates initial world data around player spawn
			void InitializeWorld(glm::vec3 spawnPoint);

			void LoadWorld(const std::string& worldName);

			// player class to later be passed in
			void UpdateWorldAroundPlayer(const glm::vec3& playerPosition);
			
		private:
			WorldGenerator* m_WorldGenerator = nullptr;
	};
}