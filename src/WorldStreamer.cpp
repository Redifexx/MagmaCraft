#include "WorldStreamer.h"
#include <memory>
#include <glm/glm.hpp>
#include "WorldManager.h"


using namespace Craft;

WorldStreamer::WorldStreamer(std::shared_ptr<NetworkManager> networkManager)
	: m_NetworkManager(networkManager)
	, m_WorldRenderer(std::make_unique<WorldRenderer>())
{
	// start with 1 chef
	unsigned int threadCount = std::thread::hardware_concurrency() / 2;
	if (threadCount < 1) threadCount = 1;

	// add a new chef to the list
	for (size_t i = 0; i < threadCount; i++)
	{
		m_Workers.emplace_back(&WorldStreamer::WorkerThread, this);
	}
}

WorldStreamer::~WorldStreamer()
{
	Shutdown();
}

void WorldStreamer::Shutdown()
{
	m_IsRunning = false;
	m_ConditionVar.notify_all(); // tell chefs shift is over

	for (auto& worker : m_Workers) // boss holding door open for chefs to leave
	{
		if (worker.joinable()) worker.join();
	}
}

void WorldStreamer::Update(float dt, const glm::vec3& playerPosition)
{
	// set own and world renderer's world manager
    if (m_WorldManager.expired())
    {
        auto networkManager = GetNetworkManager();
        if (networkManager->GetNetworkRole() != NetworkRole::NONE)
        {
			m_WorldManager = networkManager->GetWorldManager();
			if (m_WorldRenderer->m_WorldManager.expired())
			{
				m_WorldRenderer->m_WorldManager = m_WorldManager;
			}
        }
    }


	// Current Chunks
	int chunkX, chunkZ;
	GetPlayerChunkCoords(playerPosition, chunkX, chunkZ);
	glm::ivec2 curChunkPos = glm::ivec2(chunkX, chunkZ);

	if (m_FirstFrame)
	{
		m_LastChunkPos = curChunkPos;
		m_FirstFrame = false;
	}

	glm::ivec2 chunkDelta = curChunkPos - m_LastChunkPos;

	int chunkRequestsSentThisFrame = 0;

	std::shared_ptr<WorldManager> worldManager = GetWorldManager();
	std::shared_ptr<NetworkManager> networkManager = GetNetworkManager();
	if (!worldManager || !networkManager) return;

	// Remove chunks outside of render distance from local and remote chunk buffer

	if (chunkDelta.x != 0 || chunkDelta.y != 0)
	{
		// handle lag/teleport
		if (glm::length(glm::vec2(chunkDelta)) > 2.0f)
		{
			// handle eventaully
		}
		else
		{
			RemoveOldChunks(curChunkPos, m_LastChunkPos, chunkDelta);
		}
		m_LastChunkPos = curChunkPos;
	}

	// serve cooked chunks
	{
		std::vector<CookedChunk> cookedChunks;
		{
			std::lock_guard<std::mutex> lock(m_ResultMutex);
			if (!m_CookedChunks.empty())
			{
				cookedChunks = std::move(m_CookedChunks);
				m_CookedChunks.clear();
			}
		}

		// send to the gpu
		for (auto& batch : cookedChunks)
		{
			// check if order is still wanted (player went away)
			glm::ivec2 key = { batch.x, batch.z };
			if (m_ChunkBuffer.find(key) == m_ChunkBuffer.end()) continue;


			// generate the mesh
			if (!batch.vertices.empty())
			{
				auto mesh = std::make_unique<Magma::Mesh>(std::move(batch.vertices), std::move(batch.indices));
				m_WorldRenderer->AddMeshToDrawPool(std::move(mesh), key);
			}

			// FIX
			m_ChunkBuffer[key]->isLoaded = true;
			m_ChunkBuffer[key]->isPending = false;
		}
	}


	// Add chunks isnide of render distance to local and remote chunk buffer
	for (int x = (-m_ChunkRenderDistance); x <= m_ChunkRenderDistance; x++)
	{
		for (int z = (-m_ChunkRenderDistance); z <= m_ChunkRenderDistance; z++)
		{
			int curChunkX = chunkX + x;
			int curChunkZ = chunkZ + z;
			glm::ivec2 chunkKey = glm::ivec2(curChunkX, curChunkZ);

			if (m_ChunkBuffer.find(chunkKey) == m_ChunkBuffer.end())
			{
				m_ChunkBuffer[chunkKey] = std::make_unique<RenderChunk>();
			}
			RenderChunk* renderChunk = m_ChunkBuffer[chunkKey].get();

			// if it's already loaded skip
			if (renderChunk->isLoaded) continue;

			// already cooking
			if (renderChunk->isCooking) continue;

			if (worldManager->HasChunkInBuffer(curChunkX, curChunkZ))
			{
				// send to worker thread
				{
					std::lock_guard<std::mutex> lock(m_QueueMutex);
					m_JobQueue.push({ curChunkX, curChunkZ });
				}
				m_ConditionVar.notify_one();

				renderChunk->isCooking = true;
			}
			else if (!renderChunk->isPending && chunkRequestsSentThisFrame < MAX_CHUNK_REQUESTS_PER_FRAME)
			{
				networkManager.get()->RequestChunkData(
					networkManager.get()->GetClient()->GetENetPeer(),
					curChunkX,
					curChunkZ);

				// mark as pending
				renderChunk->isPending = true;

				chunkRequestsSentThisFrame++;
			}
		}
	}
}


void WorldStreamer::RemoveOldChunks(glm::ivec2 curChunkPos, glm::ivec2 lastChunkPos, glm::ivec2 chunkDelta)
{
	std::shared_ptr<WorldManager> worldManager = GetWorldManager();
	std::shared_ptr<NetworkManager> networkManager = GetNetworkManager();
	if (!worldManager || !networkManager) return;

	// check delta X
	if (chunkDelta.x != 0)
	{
		// if we move east, remove previous western most chunks
		int staleX = (chunkDelta.x > 0) ? (lastChunkPos.x - m_ChunkRenderDistance) : (lastChunkPos.x + m_ChunkRenderDistance);
		for (int z = lastChunkPos.y - m_ChunkRenderDistance; z <= lastChunkPos.y + m_ChunkRenderDistance; z++)
		{
			glm::ivec2 key = { staleX, z };

			m_ChunkBuffer.erase(key);
			
			worldManager->RemoveChunkFromBuffer(key.x, key.y);
			m_WorldRenderer->RemoveFromDrawPool(glm::ivec2(key.x, key.y));
		}
	}

	// check delta Z
	if (chunkDelta.y != 0)
	{
		// if we move east, remove previous western most chunks
		int staleZ = (chunkDelta.y > 0) ? (lastChunkPos.y - m_ChunkRenderDistance) : (lastChunkPos.y + m_ChunkRenderDistance);
		for (int x = lastChunkPos.x - m_ChunkRenderDistance; x <= lastChunkPos.x + m_ChunkRenderDistance; x++)
		{
			glm::ivec2 key = { x, staleZ };

			m_ChunkBuffer.erase(key);

			worldManager->RemoveChunkFromBuffer(key.x, key.y);
			m_WorldRenderer->RemoveFromDrawPool(glm::ivec2(key.x, key.y));
		}
	}
}

void WorldStreamer::UnloadAllChunks()
{
	std::shared_ptr<WorldManager> worldManager = GetWorldManager();
	if (!worldManager) return;

	for (auto const& [coord, renderChunk] : m_ChunkBuffer)
	{
		// remove mesh from renderer
		m_WorldRenderer->RemoveFromDrawPool(coord);

		// remove chunk from world manager buffer
		worldManager->RemoveChunkFromBuffer(coord.x, coord.y);
	}

	m_ChunkBuffer.clear();

	// reset flags
	m_FirstFrame = true;
	m_LastChunkPos = glm::ivec2(0, 0);
}

void WorldStreamer::GetPlayerChunkCoords(const glm::vec3& playerPosition, int& chunkX, int& chunkZ)
{
	chunkX = WorldToChunkPos(static_cast<int>(playerPosition.x));
	chunkZ = WorldToChunkPos(static_cast<int>(playerPosition.z));
}

void WorldStreamer::WorkerThread()
{
	while (m_IsRunning)
	{
		glm::ivec2 chunkCoord;

		// wait for an order
		{
			std::unique_lock<std::mutex> lock(m_QueueMutex);
			m_ConditionVar.wait(lock, [this]
			{
				return !m_JobQueue.empty() || !m_IsRunning;
			});

			if (!m_IsRunning) return;

			chunkCoord = m_JobQueue.front();
			m_JobQueue.pop();
		}

		// the cooking
		auto worldManager = GetWorldManager();
		if (!worldManager || !worldManager->HasChunkInBuffer(chunkCoord.x, chunkCoord.y))
		{
			// if an order was forgotten while taking too long skip it (bad customer service)
			continue;
		}

		std::shared_ptr<Chunk> chunkPtr = worldManager->GetChunkFromBuffer(chunkCoord.x, chunkCoord.y);

		if (!chunkPtr) continue;

		CookedChunk cookedChunk;
		cookedChunk.x = chunkCoord.x;
		cookedChunk.z = chunkCoord.y;

		// keep an eye our for raw chunk
		m_WorldRenderer->GenerateMesh(cookedChunk.vertices, cookedChunk.indices, chunkPtr.get(), chunkCoord);

		// serve the order
		{
			std::lock_guard<std::mutex> lock(m_ResultMutex);
			m_CookedChunks.push_back(std::move(cookedChunk));
		}
	}
}