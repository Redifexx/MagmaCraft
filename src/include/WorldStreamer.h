#pragma once

#include "WorldRenderer.h"
#include "NetworkManager.h"
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
#include "Chunk.h"
#include <map>
#include <utility>

// Streams world data based on player position
// Network Manager -> World Manager -> World Generator
// World Streamer -> (Reference) Network Manager
//                -> World Renderer
// The game layer will own both
namespace Craft
{
	struct SingleChunk
	{
		std::unique_ptr<Chunk> chunkPtr = nullptr;
		bool isLoaded = false;
	};

	class WorldStreamer
	{
		public:
			WorldStreamer(std::shared_ptr<NetworkManager> networkManager);
			void Update(float dt, const glm::vec3& playerPosition);

			void GetPlayerChunkCoords(const glm::vec3& playerPosition, int& chunkX, int& chunkZ);

			void SetChunkRenderDistance(uint8_t distance) { m_ChunkRenderDistance = distance; }

		private:
			std::unique_ptr<WorldRenderer> m_WorldRenderer;
			std::weak_ptr<NetworkManager> m_NetworkManager;
			std::weak_ptr<WorldManager> m_WorldManager;

			uint8_t m_ChunkRenderDistance = 8;
	};
}