#pragma once

#include <string>
#include <vector>
#include "Chunk.h"
#include "WorldGenerator.h"
#include "NetworkManager.h"
#include "Datatypes/EntityWorld.h"
#include "Datatypes/Components/PlayerComponent.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <memory>

// Manages world data, including loading, saving, and updating chunks
namespace Craft
{
	struct SerializedPlayerData;

	#pragma pack(push, 1)
	struct ChunkFileHeader
	{
		const uint32_t magic = 0X4D43484B; // Magma Chunk 'MCHK' Magic Number
		int32_t chunkX;
		int32_t chunkZ;
	};
	#pragma pack(pop)

	#pragma pack(push, 1)
	struct WorldFileHeader
	{
		const uint32_t magic = 0X4D435744; // Magma Craft World 'MCWD' Magic Number
		uint32_t seed;
		char worldName[32];
	};
	#pragma pack(pop)

	// only applies to file, not packet
	#pragma pack(push, 1)
	struct PlayerFileHeader
	{
		const uint32_t magic = 0X4D43504C; // Magma Craft Player 'MCPL' Magic Number
	};
	#pragma pack(pop)

	inline int WorldToChunkPos(int coord)
	{
		// maps negative chunks correctly
		return (coord >= 0) ? (coord / CHUNK_WIDTH) : ((coord - CHUNK_WIDTH + 1) / CHUNK_WIDTH);
	}

	class WorldManager
	{
		public:
			// --- WORLD CREATION/INITIALIZATION ---
			// Sets up a new world generator & world folder
			// Only ever called if server
			void CreateWorld(std::string& worldName, int seed, EntityWorld& eWorld);
			bool LoadWorld(const char* filepath);
			void SaveWorld(EntityWorld& eWorld); // saves metadata

			// Player Functions
			std::string GetPlayerFolder();
			void SavePlayerData(EntityWorld& eWorld, uint32_t entityID);
			bool LoadPlayerData(std::string& username, SerializedPlayerData& outData);
			uint32_t CreatePlayerEntity(EntityWorld& eWorld, uint8_t networkID);

			// Generates initial world data around player spawn
			// Only ever called if server
			void InitializeWorld(glm::vec3 spawnPoint);

			// --- CHUNK LOADING/SAVING ---
			void CompressChunkData(const Chunk& chunk, std::vector<uint8_t>& compressedData);
			void DecompressChunkData(const std::vector<uint8_t>& compressedData, Chunk& chunk);
			
			std::vector<uint8_t> GetChunkDataCompressed(int chunkX, int chunkZ);

			// Calls on world generator to create a chunk, then saves to file
			void CreateChunk(Chunk& chunk, int chunkX, int chunkZ);
			void SaveChunkToFile(const Chunk& chunk, int chunkX, int chunkZ);

			bool LoadChunkFromFile(std::vector<uint8_t>& compressedData, int chunkX, int chunkZ);
			bool LoadChunkFromFileDecompressed(Chunk& chunk, int chunkX, int chunkZ);

			// --- CHUNK BUFFER ---
			bool HasChunkInBuffer(int chunkX, int chunkZ) { return m_ChunkBuffer.count({ chunkX, chunkZ }); }
			Chunk* GetChunkFromBuffer(int chunkX, int chunkZ) { return m_ChunkBuffer[{chunkX, chunkZ}].get(); }
			void AddChunkToBuffer(int chunkX, int chunkZ);
			void RemoveChunkFromBuffer(int chunkX, int chunkZ);

			// data packets
			void AddChunkDataToBuffer(std::unique_ptr<Chunk> chunk, int chunkX, int chunkZ);

			// Returns block type
			const uint32_t GetBlockNeighborData(uint32_t id, Chunk* chunk, glm::ivec2 chunkPos, Direction direction);
			
			void SetNetworkIDToNameMap(std::shared_ptr<std::unordered_map <uint8_t, std::string>> map) { m_NetworkIDToNameMap = map; }
			
		private:
			std::unique_ptr<WorldGenerator> m_WorldGenerator;
			std::string m_WorldName = "New World";
			uint8_t m_ChunkRenderDistance = 8; // allocated for each client in the server
			std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>> m_ChunkBuffer;

			std::weak_ptr<std::unordered_map <uint8_t, std::string>> m_NetworkIDToNameMap;
	};
}