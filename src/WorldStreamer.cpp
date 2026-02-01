#include "WorldStreamer.h"
#include <memory>
#include <glm/glm.hpp>


using namespace Craft;

WorldStreamer::WorldStreamer(std::shared_ptr<NetworkManager> networkManager)
	: m_NetworkManager(networkManager)
	, m_WorldRenderer(std::make_unique<WorldRenderer>())
{}

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
			// handle
		}
		else
		{
			RemoveOldChunks(curChunkPos, m_LastChunkPos, chunkDelta);
		}
		m_LastChunkPos = curChunkPos;
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

			if (worldManager->HasChunkInBuffer(curChunkX, curChunkZ))
			{
				Chunk* chunk = worldManager->GetChunkFromBuffer(curChunkX, curChunkZ);

				m_WorldRenderer->RenderChunk(std::move(chunk), glm::ivec2(curChunkX, curChunkZ));
				renderChunk->isLoaded = true;
				renderChunk->isPending = false;
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