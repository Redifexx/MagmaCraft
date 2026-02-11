#pragma once

#include <vector>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "Chunk.h"
#include "Core/Mesh.h"
#include <memory>
#include <map>
#include <cstdint>
#include "WorldManager.h"

// Receives chunks from server
// Caches a number of chunks around the player
// Renders cached chunks
namespace Craft
{
	class WorldManager;

	class WorldRenderer
	{
		public:
			void GenerateMesh(std::vector<Magma::Vertex>& vertices, std::vector<uint32_t>& indices, Chunk* chunk, glm::ivec3 chunkPos);

			bool AddMeshToDrawPool(std::unique_ptr<Magma::Mesh> mesh, glm::ivec3 chunkPos);
			void RemoveFromDrawPool(glm::ivec3 chunkPos);

			void DrawWorld();

			// temporary 
			std::weak_ptr<WorldManager> m_WorldManager;
		private:
			std::unordered_map<glm::ivec3, std::unique_ptr<Magma::Mesh>> m_DrawPool;
	};
};