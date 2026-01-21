#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Chunk.h"
#include "Mesh.h"
#include <memory>
#include <cstdint>

// Receives chunks from server
// Caches a number of chunks around the player
// Renders cached chunks
namespace Craft
{
	struct ChunkMesh
	{
		std::unique_ptr<Magma::Mesh> mesh;
		glm::ivec2 chunkPos;
	};

	class WorldRenderer
	{
		public:
			WorldRenderer();
			void RenderChunk(Chunk* chunk, glm::ivec2 chunkPos);
			void GenerateMesh(std::vector<Magma::Vertex>& vertices, std::vector<uint32_t>& indices, Chunk* chunk, glm::ivec2 chunkPos);

			void DrawWorld();
		private:
			std::vector<std::unique_ptr<ChunkMesh>> m_DrawPool;
	};
}