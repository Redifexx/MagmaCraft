#pragma once

#include <vector>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "Chunk.h"
#include "Mesh.h"
#include <memory>
#include <map>
#include <cstdint>

// Receives chunks from server
// Caches a number of chunks around the player
// Renders cached chunks
namespace Craft
{

	class WorldRenderer
	{
		public:
			WorldRenderer();
			void RenderChunk(Chunk* chunk, glm::ivec2 chunkPos);
			void GenerateMesh(std::vector<Magma::Vertex>& vertices, std::vector<uint32_t>& indices, Chunk* chunk, glm::ivec2 chunkPos);

			void RemoveFromDrawPool(glm::ivec2 chunkPos);

			void DrawWorld();

		private:
			std::unordered_map<glm::ivec2, std::unique_ptr<Magma::Mesh>> m_DrawPool;
	};
};