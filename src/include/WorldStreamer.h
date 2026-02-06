#pragma once

#include "WorldRenderer.h"
#include "NetworkManager.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <vector>
#include <cstdint>
#include "Chunk.h"
#include <map>
#include <utility>
#include <memory>
// inclues for multithreading
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <atomic>


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
		bool isLoaded = false; // waits to be loaded
		bool isPending = false; // waits to be sent
		bool isCooking = false; // waits to be cooked
	};

	// gonna use restaurant vocab here
	struct CookedChunk // holds the raw chunk data cooked by thread
	{
		int x, z;
		std::unique_ptr<Chunk> chunkPtr = nullptr;
		std::vector<Magma::Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	class WorldStreamer
	{
		public:
			WorldStreamer(std::shared_ptr<NetworkManager> networkManager);
			~WorldStreamer();
			void Update(float dt, const glm::vec3& playerPosition);
			void RemoveOldChunks(glm::ivec2 curChunkPos, glm::ivec2 lastChunkPos, glm::ivec2 chunkDelta);
			void UnloadAllChunks();

			void GetPlayerChunkCoords(const glm::vec3& playerPosition, int& chunkX, int& chunkZ);

			WorldRenderer* GetWorldRenderer() const { return m_WorldRenderer.get(); }

			void SetChunkRenderDistance(uint8_t distance) { m_ChunkRenderDistance = distance; }

			void Shutdown();

		private:
			std::unique_ptr<WorldRenderer> m_WorldRenderer;
			std::weak_ptr<NetworkManager> m_NetworkManager;
			std::weak_ptr<WorldManager> m_WorldManager;
			std::shared_ptr<WorldManager> GetWorldManager() { return m_WorldManager.lock(); }
			std::shared_ptr<NetworkManager> GetNetworkManager() { return m_NetworkManager.lock(); }

			uint8_t m_ChunkRenderDistance = 8;
			glm::ivec2 m_LastChunkPos;
			bool m_FirstFrame = true;
			const int MAX_CHUNK_REQUESTS_PER_FRAME = 3;

			// Any chunk in this buffer gets rendered
			std::unordered_map<glm::ivec2, std::unique_ptr<RenderChunk>> m_ChunkBuffer;

			// Multithreading section
			void WorkerThread(); // chef's kitchen
			std::vector<std::thread> m_Workers; // chefs
			std::queue<glm::ivec2> m_JobQueue; // list of orders (coords) waiting to be cooked

			std::mutex m_QueueMutex; // protects m_JobQueue
			std::condition_variable m_ConditionVar; // calls on chef when order arrives
			std::atomic<bool> m_IsRunning = true;
			std::mutex m_ResultMutex; // servers waiting for orders to be cooked
			std::vector<CookedChunk> m_CookedChunks; 

	};
}