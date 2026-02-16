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

	RebuildChunkOffsets();
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
	int chunkX, chunkY, chunkZ;
	GetPlayerChunkCoords(playerPosition, chunkX, chunkY, chunkZ);
	glm::ivec3 curChunkPos = glm::ivec3(chunkX, chunkY, chunkZ);

	if (m_FirstFrame)
	{
		m_LastChunkPos = curChunkPos;
		m_FirstFrame = false;
	}

	glm::ivec3 chunkDelta = curChunkPos - m_LastChunkPos;

	int chunkRequestsSentThisFrame = 0;

	std::shared_ptr<WorldManager> worldManager = GetWorldManager();
	std::shared_ptr<NetworkManager> networkManager = GetNetworkManager();
	if (!worldManager || !networkManager) return;

	// Remove chunks outside of render distance from local and remote chunk buffer

	if (chunkDelta.x != 0 || chunkDelta.y != 0 || chunkDelta.z != 0)
	{
		// handle lag/teleport
		if (glm::length(glm::vec3(chunkDelta)) > 2.0f)
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
			glm::ivec3 key = { batch.x, batch.y, batch.z };
			if (m_ChunkBuffer.find(key) == m_ChunkBuffer.end()) continue;


			// generate the mesh
			if (!batch.verticesD.empty())
			{
				auto dmesh = std::make_unique<Magma::Mesh>(std::move(batch.verticesD), std::move(batch.indicesD));
				m_WorldRenderer->AddMeshToDeferredDrawPool(std::move(dmesh), key);
			}

			if (!batch.verticesF.empty())
			{
				auto fmesh = std::make_unique<Magma::Mesh>(std::move(batch.verticesF), std::move(batch.indicesF));
				m_WorldRenderer->AddMeshToForwardDrawPool(std::move(fmesh), key);
			}

			m_ChunkBuffer[key]->isLoaded = true;
			m_ChunkBuffer[key]->isPending = false;
			m_ChunkBuffer[key]->isCooking = false;
		}
	}

	// handle requests that are stuck on pending
	for (auto& [key, renderChunk] : m_ChunkBuffer)
	{
		if (renderChunk->isPending)
		{
			renderChunk->pendingTimer += dt;
			if (renderChunk->pendingTimer > 3.0f) // test with 3 sec timout
			{
				renderChunk->isPending = false;
				renderChunk->pendingTimer = 0.0f;
			}
		}
	}

	// process queue
	for (const auto& chunkOffset : m_SortedChunkOffsets)
	{
		if (chunkRequestsSentThisFrame >= MAX_CHUNK_REQUESTS_PER_FRAME) break;

		int curChunkX = chunkX + chunkOffset.x;
		int curChunkY = chunkY + chunkOffset.y;
		int curChunkZ = chunkZ + chunkOffset.z;
		glm::ivec3 chunkKey = { curChunkX, curChunkY, curChunkZ };

		if (m_ChunkBuffer.find(chunkKey) == m_ChunkBuffer.end())
		{
			m_ChunkBuffer[chunkKey] = std::make_unique<RenderChunk>();
		}

		RenderChunk* renderChunk = m_ChunkBuffer[chunkKey].get();

		// if it's already loaded skip
		if (renderChunk->isLoaded || renderChunk->isCooking) continue;

		if (worldManager->HasChunkInBuffer(curChunkX, curChunkY, curChunkZ))
		{
			renderChunk->isPending = false;

			// send to worker thread
			{
				std::lock_guard<std::mutex> lock(m_QueueMutex);
				m_JobQueue.push({ curChunkX, curChunkY, curChunkZ });
			}
			m_ConditionVar.notify_one(); // wakes up 1 worker

			renderChunk->isCooking = true;
		}
		else if (!renderChunk->isPending)
		{
			networkManager.get()->RequestChunkData(
				networkManager.get()->GetClient()->GetENetPeer(),
				curChunkX,
				curChunkY,
				curChunkZ);

			// mark as pending
			renderChunk->isPending = true;
			renderChunk->pendingTimer = 0.0f;
			chunkRequestsSentThisFrame++;
		}
	}
}


void WorldStreamer::RemoveOldChunks(glm::ivec3 curChunkPos, glm::ivec3 lastChunkPos, glm::ivec3 chunkDelta)
{
	std::shared_ptr<WorldManager> worldManager = GetWorldManager();
	std::shared_ptr<NetworkManager> networkManager = GetNetworkManager();
	if (!worldManager || !networkManager) return;

	// check delta X
	if (chunkDelta.x != 0)
	{
		// if we move east, remove previous western most chunks
		int staleX = (chunkDelta.x > 0) ? (lastChunkPos.x - m_ChunkRenderDistance) : (lastChunkPos.x + m_ChunkRenderDistance);
		for (int y = lastChunkPos.y - m_ChunkRenderDistance; y <= lastChunkPos.y + m_ChunkRenderDistance; y++)
		{
			for (int z = lastChunkPos.z - m_ChunkRenderDistance; z <= lastChunkPos.z + m_ChunkRenderDistance; z++)
			{
				glm::ivec3 key = { staleX, y, z };

				m_ChunkBuffer.erase(key);

				worldManager->RemoveChunkFromBuffer(key.x, key.y, key.z);
				m_WorldRenderer->RemoveFromDrawPools(glm::ivec3(key.x, key.y, key.z));
			}
		}
	}

	// check delta Y
	if (chunkDelta.y != 0)
	{
		// if we move east, remove previous western most chunks
		int staleY = (chunkDelta.y > 0) ? (lastChunkPos.y - m_ChunkRenderDistance) : (lastChunkPos.y + m_ChunkRenderDistance);
		for (int x = lastChunkPos.x - m_ChunkRenderDistance; x <= lastChunkPos.x + m_ChunkRenderDistance; x++)
		{
			for (int z = lastChunkPos.z - m_ChunkRenderDistance; z <= lastChunkPos.z + m_ChunkRenderDistance; z++)
			{
				glm::ivec3 key = { x, staleY, z };

				m_ChunkBuffer.erase(key);

				worldManager->RemoveChunkFromBuffer(key.x, key.y, key.z);
				m_WorldRenderer->RemoveFromDrawPools(glm::ivec3(key.x, key.y, key.z));
			}
		}
	}

	// check delta Z
	if (chunkDelta.z != 0)
	{
		// if we move east, remove previous western most chunks
		int staleZ = (chunkDelta.z > 0) ? (lastChunkPos.z - m_ChunkRenderDistance) : (lastChunkPos.z + m_ChunkRenderDistance);
		for (int y = lastChunkPos.y - m_ChunkRenderDistance; y <= lastChunkPos.y + m_ChunkRenderDistance; y++)
		{
			for (int x = lastChunkPos.x - m_ChunkRenderDistance; x <= lastChunkPos.x + m_ChunkRenderDistance; x++)
			{
				glm::ivec3 key = { x, y, staleZ };

				m_ChunkBuffer.erase(key);

				worldManager->RemoveChunkFromBuffer(key.x, key.y, key.z);
				m_WorldRenderer->RemoveFromDrawPools(glm::ivec3(key.x, key.y, key.z));
			}
		}
	}
}

void WorldStreamer::UnloadAllChunks()
{
	std::cout << "Unloading all chunks..." << std::endl;
	std::shared_ptr<WorldManager> worldManager = GetWorldManager();
	if (!worldManager) return;

	for (auto const& [coord, renderChunk] : m_ChunkBuffer)
	{
		// remove mesh from renderer
		m_WorldRenderer->RemoveFromDrawPools(coord);

		// remove chunk from world manager buffer
		worldManager->RemoveChunkFromBuffer(coord.x, coord.y, coord.z);
	}

	m_ChunkBuffer.clear();

	// reset flags
	m_FirstFrame = true;
	m_LastChunkPos = glm::ivec3(0, 0, 0);
	std::cout << "All chunks unloaded." << std::endl;
}

void WorldStreamer::GetPlayerChunkCoords(const glm::vec3& playerPosition, int& chunkX, int& chunkY, int& chunkZ)
{
	chunkX = WorldToChunkPos(static_cast<int>(playerPosition.x));
	chunkY = WorldToChunkPos(static_cast<int>(playerPosition.y));
	chunkZ = WorldToChunkPos(static_cast<int>(playerPosition.z));
}

void WorldStreamer::RebuildChunkOffsets()
{
	m_SortedChunkOffsets.clear();

	for (int x = -m_ChunkRenderDistance; x <= m_ChunkRenderDistance; x++)
	{
		for (int y = -m_ChunkRenderDistance; y <= m_ChunkRenderDistance; y++)
		{
			for (int z = -m_ChunkRenderDistance; z <= m_ChunkRenderDistance; z++)
			{
				m_SortedChunkOffsets.push_back({ x, y, z });
			}
		}
	}

	// sort once here
	std::sort(m_SortedChunkOffsets.begin(), m_SortedChunkOffsets.end(),
	[](const glm::ivec3& a, const glm::ivec3& b) {
		return (a.x * a.x + a.y * a.y + a.z * a.z) < (b.x * b.x + b.y * b.y + b.z * b.z);
	});
}


void WorldStreamer::WorkerThread()
{
	while (m_IsRunning)
	{
		glm::ivec3 chunkCoord;

		// wait for an order
		{
			std::unique_lock<std::mutex> lock(m_QueueMutex);
			// chef sleeps until bell wakes it up or kitchen is closing
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
		if (!worldManager || !worldManager->HasChunkInBuffer(chunkCoord.x, chunkCoord.y, chunkCoord.z))
		{
			// if an order was forgotten while taking too long skip it (bad customer service)
			continue;
		}

		std::shared_ptr<Chunk> chunkPtr = worldManager->GetChunkFromBuffer(chunkCoord.x, chunkCoord.y, chunkCoord.z);

		if (!chunkPtr) continue;

		CookedChunk cookedChunk;
		cookedChunk.x = chunkCoord.x;
		cookedChunk.y = chunkCoord.y;
		cookedChunk.z = chunkCoord.z;

		// keep an eye our for raw chunk
		m_WorldRenderer->GenerateMeshes(
			cookedChunk.verticesD,
			cookedChunk.indicesD,
			cookedChunk.verticesF,
			cookedChunk.indicesF,
			chunkPtr.get(),
			chunkCoord);

		// serve the order
		{
			std::lock_guard<std::mutex> lock(m_ResultMutex);
			m_CookedChunks.push_back(std::move(cookedChunk));
		}
	}
}