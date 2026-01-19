#include "WorldStreamer.h"
#include <memory>


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

	int chunkRequestsSentThisFrame = 0;

	std::shared_ptr<WorldManager> worldManager = GetWorldManager();
	std::shared_ptr<NetworkManager> networkManager = GetNetworkManager();
	if (!worldManager || !networkManager) return;

	for (int i = (-m_ChunkRenderDistance); i <= m_ChunkRenderDistance; i++)
	{
		for (int j = (-m_ChunkRenderDistance); j <= m_ChunkRenderDistance; j++)
		{
			int curChunkX = chunkX + i;
			int curChunkZ = chunkZ + j;
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
				// generate mesh here
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
				}
			}
		}
	}
}

void WorldStreamer::GetPlayerChunkCoords(const glm::vec3& playerPosition, int& chunkX, int& chunkZ)
{
	chunkX = static_cast<int>(playerPosition.x) / CHUNK_WIDTH;
	chunkZ = static_cast<int>(playerPosition.z) / CHUNK_WIDTH;
}