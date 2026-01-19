#pragma once

#include "WorldRenderer.h"
#include "NetworkManager.h"
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
#include "Chunk.h"
#include <map>
#include <utility>
#include <memory>

// Streams world data based on player position
// Network Manager -> World Manager -> World Generator
// World Streamer -> (Reference) Network Manager
//                -> World Renderer
// The game layer will own both
namespace Craft
{
	struct RenderChunk
	{
		std::unique_ptr<Chunk> chunkPtr = nullptr;
		bool isLoaded = false;
		bool isPending = false;
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
			std::shared_ptr<WorldManager> GetWorldManager() { return m_WorldManager.lock(); }
			std::shared_ptr<NetworkManager> GetNetworkManager() { return m_NetworkManager.lock(); }

			uint8_t m_ChunkRenderDistance = 8;
			const int MAX_CHUNK_REQUESTS_PER_FRAME = 3;

			// Any chunk in this buffer gets rendered
			std::unordered_map<glm::ivec2, std::unique_ptr<RenderChunk>> m_ChunkBuffer;
	};
}