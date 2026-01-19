#include "WorldStreamer.h"

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

	for (int i = (-m_ChunkRenderDistance); i <= m_ChunkRenderDistance; i++)
	{
		for (int j = (-m_ChunkRenderDistance); j <= m_ChunkRenderDistance; j++)
		{
			SingleChunk& singleChunk = m_NetworkManager->GetWorldManager()->m_ChunkBuffer[std::make_pair(chunkX + i, chunkZ + j)];
			if (!singleChunk.isLoaded)
			{
				if (m_NetworkManager->GetNetworkRole() == NetworkRole::CLIENT)
				{
					m_NetworkManager->RequestChunkData(m_NetworkManager->GetClient()->GetENetPeer(), chunkX + i, chunkZ + j);
				}
			}
		}
	}

	// Check buffer

}

void WorldStreamer::GetPlayerChunkCoords(const glm::vec3& playerPosition, int& chunkX, int& chunkZ)
{
	chunkX = static_cast<int>(playerPosition.x) / CHUNK_WIDTH;
	chunkZ = static_cast<int>(playerPosition.z) / CHUNK_WIDTH;
}