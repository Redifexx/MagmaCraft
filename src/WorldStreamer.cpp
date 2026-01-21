#include "WorldStreamer.h"
#include <memory>
#include <glm/glm.hpp>


using namespace Craft;

WorldStreamer::WorldStreamer(std::shared_ptr<NetworkManager> networkManager)
	: m_NetworkManager(networkManager)
	, m_WorldRenderer(std::make_unique<WorldRenderer>())
{
	m_WorldManager = networkManager->GetWorldManager();
}

void WorldStreamer::Update(float dt, const glm::vec3& playerPosition)
{
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
				NetworkRole role = networkManager->GetNetworkRole();

				if (role == NetworkRole::CLIENT)
				{
					networkManager.get()->RequestChunkData(
						networkManager.get()->GetClient()->GetENetPeer(),
						curChunkX,
						curChunkZ);

					// mark as pending
					renderChunk->isPending = true;

					chunkRequestsSentThisFrame++;
				}
				else if (role == NetworkRole::SERVER) // until i made a dedicated server, server == server + client
				{
					// skip localhost, send data directly
					worldManager->AddChunkToBuffer(curChunkX, curChunkZ);
					renderChunk->isPending = true;
				}
			}
		}
	}
}

// finish bro
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

void WorldStreamer::GetPlayerChunkCoords(const glm::vec3& playerPosition, int& chunkX, int& chunkZ)
{
	chunkX = static_cast<int>(playerPosition.x) / CHUNK_WIDTH;
	chunkZ = static_cast<int>(playerPosition.z) / CHUNK_WIDTH;
}