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
			// i have to generate seperate meshes for opaque and transparent objects
			void GenerateMeshes(
				std::vector<Magma::Vertex>& verticesD,
				std::vector<uint32_t>& indicesD,
				std::vector<Magma::Vertex>& verticesF,
				std::vector<uint32_t>& indicesF,
				Chunk* chunk,
				glm::ivec3 chunkPos
			);

			// normal meshes
			bool AddMeshToDeferredDrawPool(std::unique_ptr<Magma::Mesh> mesh, glm::ivec3 chunkPos);
			void RemoveFromDeferredDrawPool(glm::ivec3 chunkPos);

			// everything with transparency
			bool AddMeshToForwardDrawPool(std::unique_ptr<Magma::Mesh> mesh, glm::ivec3 chunkPos);
			void RemoveFromForwardDrawPool(glm::ivec3 chunkPos);

			bool AddMeshesToDrawPool(std::unique_ptr<Magma::Mesh> dmesh, std::unique_ptr<Magma::Mesh> fmesh, glm::ivec3 chunkPos);
			void RemoveFromDrawPools(glm::ivec3 chunkPos);

			void DrawDeferredWorld();
			void DrawForwardWorld(glm::vec3 camPos);

			// temporary 
			std::weak_ptr<WorldManager> m_WorldManager;
		private:
			std::unordered_map<glm::ivec3, std::unique_ptr<Magma::Mesh>> m_DeferredDrawPool;
			std::unordered_map<glm::ivec3, std::unique_ptr<Magma::Mesh>> m_ForwardDrawPool;
	};
};